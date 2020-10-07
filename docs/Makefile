# makefile for Sphinx documentation
#
#

#$(srctree)
srctree := $(abspath ./..)

# You can set these variables from the command line.
LSPHINXOPTS    =
BUILDDIR = build
SPHINXBUILD   = sphinx-build
PDFLATEX      = xelatex
LATEXOPTS     = -interaction=batchmode
#AUTODOC_FOLDER := $(abspath ./thinkos)
KERNELDOC       = $(srctree)/scripts/kernel-doc
#KERNELDOC_CONF  = -D kerneldoc_srctree=$(srctree) -D kerneldoc_bin=$(KERNELDOC)
#KERNELDOC_CONF  = -D kerneldoc_srctree=$(srctree) 
#KERNELDOC_CONF  = -D srctree=$(srctree) 
ALLSPHINXOPTS   =  $(KERNELDOC_CONF) $(SPHINXOPTS)
#

export srctree 

all: arm-html user-html

html: user-html kernel-html arm-html

pdf: arm.pdf

user-html:
	$(SPHINXBUILD) -M html "user" "$(BUILDDIR)/user" $(ALLSPHINXOPTS) $(O)

kernel-html:
	@$(SPHINXBUILD) -M html "kernel" "$(BUILDDIR)/kernel" $(SPHINXOPTS) $(O)
	
arm-html:
	@$(SPHINXBUILD) -M html "arm" "$(BUILDDIR)/arm" $(SPHINXOPTS) $(O)

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

latex:
	@$(SPHINXBUILD) -M latex "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

pdf: latex
	$(MAKE) PDFLATEX=$(PDFLATEX) LATEXOPTS="$(LATEXOPTS)" -C $(BUILDDIR)/latex

.PHONY: help Makefile

