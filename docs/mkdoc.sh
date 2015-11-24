#!/bin/sh

rm -rf xml/*
rm -rf html/*

DOC_MODULE=thinkos
SRC_DIR=src
#SRC_DIR=../sdk/libthinkos

#gtkdoc-scan --module ${DOC_MODULE} ${SRC_DIR} ../sdk/include/thinkos.h
#gtkdoc-scan --module ${DOC_MODULE} --rebuild-sections --source-dir=${SRC_DIR} 
gtkdoc-scan --module ${DOC_MODULE} --source-dir=${SRC_DIR} 
gtkdoc-mkdb --module ${DOC_MODULE} --output-format=xml --source-dir=${SRC_DIR}

mkdir html 2> /dev/null

cd html 

gtkdoc-mkhtml ${DOC_MODULE} ../thinkos-docs.xml

cd ..

#gtkdoc-fixxref --module=${DOC_MODULE} --module-dir=html

