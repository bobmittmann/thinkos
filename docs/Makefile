# makefile for Sphinx documentation
#
#

#
# LinuxDOC source code base path 
srctree := $(abspath ./..)
export srctree 

SPHINXOPTS =
BUILDDIR = build
SPHINXBUILD = sphinx-build
PDFLATEX = xelatex
LATEXOPTS = -interaction=batchmode

all: html 

help:
	@echo "Please use 'make target' where target is one of"
	@echo "    html        to make standalone HTML files"
	@echo "    htmlhelp    to make HTML files and an HTML help project"
	@echo "    devhelp     to make HTML files and a Devhelp project"
	@echo "    epub        to make an epub"
	@echo "    man         to make manual pages"
	@echo "    texinfo     to make Texinfo files"
	@echo "    latex       to make LaTeX files"
	@echo "    pdf         to make PDF documents"

html: user-html arm-html

htmlhelp: arm-htmlhelp user-htmlhelp

devhelp: arm-devhelp user-devhelp

man: arm-man user-man

texinfo: arm-texinfo user-texinfo

epub: arm-epub user-epub

latex: arm-latex user-latex

pdf: arm-pdf user-pdf

%-html:
	@$(SPHINXBUILD) -M html "$(@:%-html=%)" "$(BUILDDIR)/$(@:%-html=%)" $(SPHINXOPTS) $(O)

%-htmlhelp:
	@$(SPHINXBUILD) -M htmlhelp "$(@:%-htmlhelp=%)" "$(BUILDDIR)/$(@:%-htmlhelp=%)" $(SPHINXOPTS) $(O)

%-devhelp:
	@$(SPHINXBUILD) -M devhelp "$(@:%-devhelp=%)" "$(BUILDDIR)/$(@:%-devhelp=%)" $(SPHINXOPTS) $(O)

%-epub:
	@$(SPHINXBUILD) -M epub "$(@:%-epub=%)" "$(BUILDDIR)/$(@:%-epub=%)" $(SPHINXOPTS) $(O)

%-man:
	@$(SPHINXBUILD) -M man "$(@:%-man=%)" "$(BUILDDIR)/$(@:%-man=%)" $(SPHINXOPTS) $(O)

%-texinfo:
	@$(SPHINXBUILD) -M texinfo "$(@:%-texinfo=%)" "$(BUILDDIR)/$(@:%-texinfo=%)" $(SPHINXOPTS) $(O)

%-latex:
	@$(SPHINXBUILD) -M latex "$(@:%-latex=%)" "$(BUILDDIR)/$(@:%-latex=%)" $(SPHINXOPTS) $(O)

%-pdf: %-latex
	$(MAKE) PDFLATEX=$(PDFLATEX) LATEXOPTS="$(LATEXOPTS)" -C $(BUILDDIR)/$(@:%-pdf=%)/latex


.PHONY: help Makefile

