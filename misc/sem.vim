" Vim 7 syntax file 
" Language    : SIMPLESEM 
" Maintainer  : Davide Angelocola <davide.angelocola@gmail.com>

" keywords
syn keyword semStatement  set jump jumpt halt
syn keyword semRegister   read write writeln 

" comments
syn match   semComment 	"#.*$"

" constants
syn keyword semConstant D ip	
 
" strings
syn match   semSpecial	contained "\\[nrtvfba]"
syn region  semString 	start=+'+  end=+'+ skip=+\\\\\|\\'+ contains=Special
syn region  semString	start=+"+  end=+"+ skip=+\\\\\|\\"+ contains=Special
syn region  semString 	start=+\[\[+ end=+\]\]+

" integers 
syn match   semNumber 	"\<[0-9]\+\>"

" default highlightings
hi def link semStatement Statement
hi def link semRegister Operator
hi def link semComment Comment
hi def link semConstant Special
hi def link semString String
hi def link semNumber Number
let b:current_syntax = "sem"
