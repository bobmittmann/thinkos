# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/stable/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import re
import sys, os

#import linuxdoc
sys.path.insert(0, os.path.abspath('../linuxdoc'))

# -- Project information -----------------------------------------------------
project = 'ThinkOS - Application Development Guide'
copyright = '2020, Bob Mittmann'
author = 'Bob Mittmann'
show_authors = True

# The short X.Y version
version = '0.3'
# The full version, including alpha/beta/rc tags
release = '0.3-rc1'

# -- General configuration ---------------------------------------------------

# https://return42.github.io/linuxdoc/

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
	   'sphinx.ext.imgmath'
    , 'sphinx.ext.autodoc'
    , 'sphinx.ext.extlinks'
    #, 'sphinx.ext.autosummary'
    #, 'sphinx.ext.doctest'
    , 'sphinx.ext.todo'
    , 'sphinx.ext.coverage'
    #, 'sphinx.ext.pngmath'
    #, 'sphinx.ext.mathjax'
    , 'sphinx.ext.viewcode'
    , 'sphinx.ext.intersphinx'
	, 'linuxdoc.rstFlatTable'      # 'flat-table' reST-directive.
	, 'linuxdoc.rstKernelDoc'    # 'kernel-doc' reST-directive.
	, 'linuxdoc.kernel_include'  # 'kernel-include' reST-directive.
	, 'linuxdoc.manKernelDoc'    # 'kernel-doc-man' builder
	, 'linuxdoc.cdomain'         # Replacement for the sphinx c-domain.
	, 'linuxdoc.kfigure'         # scalable image handling.
]

c_id_attributes = ["__user" "unsigned"]

# Add any paths that contain templates here, relative to this directory.
#templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path .
exclude_patterns = [ ]

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

primary_domain = 'c'
highlight_language = 'none'

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'alabaster'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
# html_theme_options = {}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
#html_static_path = ['_static']

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# The default sidebars (for documents that don't match any pattern) are
# defined by theme itself.  Builtin themes are using these templates by
# default: ``['localtoc.html', 'relations.html', 'sourcelink.html',
# 'searchbox.html']``.
#
# html_sidebars = {}


# -- Options for HTMLHelp output ---------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'ThinkOSAppDev'

try:
    import sphinx_rtd_theme
    html_theme = 'sphinx_rtd_theme'
    html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
except ImportError:
    sys.stderr.write('Warning: The Sphinx \'sphinx_rtd_theme\' HTML theme was not found. Make sure you have the theme installed to produce pretty HTML output. Falling back to the default theme.\n')


# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#html_theme_options = {}

# Add any paths that contain custom themes here, relative to this directory.
#html_theme_path = []

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
#html_title = None

# A shorter title for the navigation bar.  Default is the same as html_title.
#html_short_title = None

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
html_logo = '../img/thinkos_logo.svg'

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
html_favicon = '../img/thinkos_icon.png'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".

#html_static_path = ['sphinx-static']

#html_context = {
#    'css_files': [
#        '_static/theme_overrides.css',
#    ],
#}

# Add any extra paths that contain custom files (such as robots.txt or
# .htaccess) here, relative to this directory. These files are copied
# directly to the root of the documentation.
#html_extra_path = []

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
#html_last_updated_fmt = '%b %d, %Y'

# If true, SmartyPants will be used to convert quotes and dashes to
# typographically correct entities.
#html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
#html_sidebars = {}

# Additional templates that should be rendered to pages, maps page names to
# template names.
#html_additional_pages = {}

# If false, no module index is generated.
#html_domain_indices = True

# If false, no index is generated.
#html_use_index = True

# If true, the index is split into individual pages for each letter.
#html_split_index = False

# If true, links to the reST sources are added to the pages.
#html_show_sourcelink = True

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
html_show_sphinx = False

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
#html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
#html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
#html_file_suffix = None

# Language to be used for generating the HTML full-text search index.
# Sphinx supports the following languages:
#   'da', 'de', 'en', 'es', 'fi', 'fr', 'h', 'it', 'ja'
#   'nl', 'no', 'pt', 'ro', 'r', 'sv', 'tr'
#html_search_language = 'en'

# A dictionary with options for the search language support, empty by default.
# Now only 'ja' uses this config value
#html_search_options = {'type': 'default'}

# The name of a javascript file (relative to the configuration directory) that
# implements a search results scorer. If empty, the default will be used.
#html_search_scorer = 'scorer.js'


# ------------------------------------------------------------------------------
# Options of the kernel-doc parser
# ------------------------------------------------------------------------------

# If true, fatal errors (like missing function descripions) raise an error. If
# false, insert Oops messages for fatal errors.  Default: True
kernel_doc_raise_error = True

# Oops messages are Sphinx ``.. todo::`` directives. To inster the Oops messages
# from the kernel-doc parser we have to active todo_include_todos also.
todo_include_todos = True

# If true, more warnings will be logged. E.g. a missing description of a
# function's return value will be logged.
# Default: True
kernel_doc_verbose_warn = True

# Set parser's default kernel-doc mode ``reST|kernel-doc``.
# Default: "reST"
kernel_doc_mode = "reST"

# Global fallback for man section of kernel-doc directives. Set this value if
# you want to create man pages for those kernel-doc directives, which has not
# been set a ``:man-sect:`` value.
# Default: None
kernel_doc_mansect = None

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [ ]

# In nickpick mode, it will complain about lots of missing references that
#
# 1) are just typedefs like: bool, __u32, etc;
# 2) It will complain for things like: enum, NULL;
# 3) It will complain for symbols that should be on different
#    books (but currently aren't ported to ReST)
#
# The list below has a list of such symbols to be ignored in nitpick mode
#
nitpick_ignore = [
    ("c:type", "bool"),
    ("c:type", "enum"),
    ("c:type", "u16"),
    ("c:type", "u32"),
    ("c:type", "u64"),
    ("c:type", "u8"),
    ]

