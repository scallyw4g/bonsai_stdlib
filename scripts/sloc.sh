#! /bin/bash

SRC_SLOC=$( find src         -name \*.cpp -print -o -name \*.h -print | xargs cat | wc -l)
META_SLOC=$(find generated   -name \*.cpp -print -o -name \*.h -print | xargs cat | wc -l)
PRNT_SLOC=$(find debug_print -name \*.cpp -print -o -name \*.h -print | xargs cat | wc -l)
DEPS_SLOC=$(find external    -name \*.cpp -print -o -name \*.h -print | xargs cat | wc -l)
EXMP_SLOC=$(find examples    -name \*.cpp -print -o -name \*.h -print | xargs cat | wc -l)
SHDR_SLOC=$(find . -name \*.fragmentshader -print -o -name \*.vertexshader -print -o -name \*.glsl -print | xargs cat | wc -l)

echo ""
echo " Hand written   $SRC_SLOC"
echo " Meta output    $META_SLOC"
echo " Debug print    $PRNT_SLOC"
echo " Deps           $DEPS_SLOC"
echo " Examples       $EXMP_SLOC"
echo " Shaders        $SHDR_SLOC"
echo "--------------------------"
echo " Total          $(( SRC_SLOC + META_SLOC + DEPS_SLOC + PRNT_SLOC + EXMP_SLOC + SHDR_SLOC )) sloc"
echo ""
