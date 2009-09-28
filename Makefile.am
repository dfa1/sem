SUBDIRS = doc examples misc gnulib src
ACLOCAL_AMFLAGS = -I m4

# Avoid: .hg - hg directory
#        *~  - emacs backups
#        .#* - merge originals
#
# *~ and .#* only occur when a whole directory without it's own Makefile.am
# is distributed (such as src/os-*).
dist-hook:
	find $(distdir) \( -name .hg -type d \) -o -name "*~" -o -name ".#*" -exec rm {} \;

EXTRA_DIST = m4/gnulib-cache.m4
