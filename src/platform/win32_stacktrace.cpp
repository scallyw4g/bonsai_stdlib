// NOTE(Jesse): This is some omega-barf code I copied off of SO in a flurry of
// needing to get shit done.
// https://stackoverflow.com/questions/6205981/windows-c-stack-trace-from-a-running-app

/* I modified Jerry Coffin's code as you see below. Modifications: */

/* A. I was always missing the most important frame of all: the line that actually triggered the exception. Turns out this was because the "do" loop was moving to the next frame at the top instead of at the bottom. */

/* B. I removed the threading stuff. */

/* C. I simplified a few bits. */

/* You should cut the "WinMain()" function at the bottom and instead put the __try .. __except stuff in your own 'main/WinMain' function. Also replace 'YourMessage' with your own function to display a message box or email it or whatever. */

/* The symbolic information is stored inside a .pdb file, not the .exe. You need to give your users your .pdb file, and you need to ensure that their process can find it. See the string inside the code below - replace this with a folder that will work on the users' machine, or NULL - NULL means it will search in the process's current working directory. The .pdb file must have the exact same name on the users' computer as it had when you ran your compiler - to configure this to something different, see "Properties > Linker > Debugging > Generate Program Database File". */

/* #include <windows.h> */
#include <string>
#include <sstream>
#include <vector>
#include <Psapi.h>
#include <algorithm>
#include <iterator>


// Thanks, Jerry Coffin.

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack( push, before_imagehlp, 8 )
#include <ImageHlp.h>
#pragma pack( pop, before_imagehlp )

struct module_data {
    std::string image_name;
    std::string module_name;
    void *base_address;
    DWORD load_size;
};


DWORD DumpStackTrace( EXCEPTION_POINTERS *ep );

class symbol { 
    typedef IMAGEHLP_SYMBOL64 sym_type;
    sym_type *sym;
    static const int max_name_len = 1024;

public:
    symbol(HANDLE process, DWORD64 address) : sym((sym_type *)::operator new(sizeof(*sym) + max_name_len)) {
        memset(sym, '\0', sizeof(*sym) + max_name_len);
        sym->SizeOfStruct = sizeof(*sym);
        sym->MaxNameLength = max_name_len;
        DWORD64 displacement;

        SymGetSymFromAddr64(process, address, &displacement, sym);
    }

    std::string name() { return std::string(sym->Name); }
    std::string undecorated_name() { 
        if (*sym->Name == '\0')
            return "<couldn't map PC to fn name>";
        std::vector<char> und_name(max_name_len);
        UnDecorateSymbolName(sym->Name, &und_name[0], max_name_len, UNDNAME_COMPLETE);
        return std::string(&und_name[0], strlen(&und_name[0]));
    }
};


class get_mod_info {
    HANDLE process;
    static const int buffer_length = 4096;
public:
    get_mod_info(HANDLE h) : process(h) {}

    module_data operator()(HMODULE module) { 
        module_data ret;
        char temp[buffer_length];
        MODULEINFO mi;

        GetModuleInformation(process, module, &mi, sizeof(mi));
        ret.base_address = mi.lpBaseOfDll;
        ret.load_size = mi.SizeOfImage;

        GetModuleFileNameEx(process, module, temp, sizeof(temp));
        ret.image_name = temp;
        GetModuleBaseName(process, module, temp, sizeof(temp));
        ret.module_name = temp;
        std::vector<char> img(ret.image_name.begin(), ret.image_name.end());
        std::vector<char> mod(ret.module_name.begin(), ret.module_name.end());
        SymLoadModule64(process, 0, &img[0], &mod[0], (DWORD64)ret.base_address, ret.load_size);
        return ret;
    }
};


// if you use C++ exception handling: install a translator function
// with set_se_translator(). In the context of that function (but *not*
// afterwards), you can either do your stack dump, or save the CONTEXT
// record as a local copy. Note that you must do the stack dump at the
// earliest opportunity, to avoid the interesting stack-frames being gone
// by the time you do the dump.
DWORD DumpStackTrace(EXCEPTION_POINTERS *ep) 
{
    HANDLE process = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    int frame_number=0;
    DWORD offset_from_symbol=0;
    IMAGEHLP_LINE64 line = {};
    std::vector<module_data> modules;
    DWORD cbNeeded;
    std::vector<HMODULE> module_handles(1);

    // Load the symbols:
    // WARNING: You'll need to replace <pdb-search-path> with either NULL
    // or some folder where your clients will be able to find the .pdb file.
    if (!SymInitialize(process, NULL, false)) 
        throw(std::logic_error("Unable to initialize symbol handler"));
    DWORD symOptions = SymGetOptions();
    symOptions |= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME;
    SymSetOptions(symOptions);
    EnumProcessModules(process, &module_handles[0], module_handles.size() * sizeof(HMODULE), &cbNeeded);
    module_handles.resize(cbNeeded/sizeof(HMODULE));
    EnumProcessModules(process, &module_handles[0], module_handles.size() * sizeof(HMODULE), &cbNeeded);
    std::transform(module_handles.begin(), module_handles.end(), std::back_inserter(modules), get_mod_info(process));
    void *base = modules[0].base_address;

    // Setup stuff:
    CONTEXT* context = ep->ContextRecord;
#ifdef _M_X64
    STACKFRAME64 frame;
    frame.AddrPC.Offset = context->Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->Rsp;
    frame.AddrStack.Mode = AddrModeFlat;    
    frame.AddrFrame.Offset = context->Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
#else
    STACKFRAME64 frame;
    frame.AddrPC.Offset = context->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->Esp;
    frame.AddrStack.Mode = AddrModeFlat;    
    frame.AddrFrame.Offset = context->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
#endif
    line.SizeOfStruct = sizeof line;
    IMAGE_NT_HEADERS *h = ImageNtHeader(base);
    DWORD image_type = h->FileHeader.Machine;
    int n = 0;

    // Build the string:
    std::ostringstream builder;
    do {
        if ( frame.AddrPC.Offset != 0 ) {
            std::string fnName = symbol(process, frame.AddrPC.Offset).undecorated_name();
            builder << fnName;
            SymGetLineFromAddr64( process, frame.AddrPC.Offset, &offset_from_symbol, &line);

            if (line.FileName)
            {
              DebugLine("%32s   %s:%d", fnName.c_str(), line.FileName, line.LineNumber);
            }
            else
            {
              DebugLine("%32s   (unknown_file):%d", fnName.c_str(), line.LineNumber);
            }
        }
        else
            builder << "(No Symbols: PC == 0)";
        if (!StackWalk64(image_type, process, hThread, &frame, context, NULL, 
                            SymFunctionTableAccess64, SymGetModuleBase64, NULL))
            break;
        if (++n > 10)
            break;
    } while (frame.AddrReturn.Offset != 0);
    //return EXCEPTION_EXECUTE_HANDLER;
    SymCleanup(process);

    return EXCEPTION_EXECUTE_HANDLER;
}

link_internal void
PlatformDebugStacktrace()
{
  try { throw 0; } __except (DumpStackTrace(GetExceptionInformation())) { }
}
