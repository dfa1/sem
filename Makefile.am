SUBDIRS = src
# Avoid: .hg - hg directory
#        *~  - emacs backups
#        .#* - merge originals
#
# *~ and .#* only occur when a whole directory without it's own Makefile.am
# is distributed (such as src/os-*).
dist-hook:
	find $(distdir) \( -name .hg -type d \) -o -name "*~" -o -name ".#*" -exec rm {} \;


