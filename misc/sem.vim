" Vim syntax file
" Language    : SIMPLESEM 
" Maintainer  : Davide Angelocola <davide.angelocola@tiscali.it>
" Last Change : Mon Jan  5 14:33:12 CET 2004

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" keywords
syn keyword semStatement  set jump jumpt halt
syn keyword semRegister   read write writeln 

" comments
syn match   semComment 	"#.*$" contains=semTodo
syn keyword semTodo   	contained TODO FIXME XXX

" contants 
syn keyword semConstant D ip	
 
" strings
syn match   semSpecial	contained "\\[nrtvfba]"
syn region  semString 	start=+'+  end=+'+ skip=+\\\\\|\\'+ contains=Special
syn region  semString	start=+"+  end=+"+ skip=+\\\\\|\\"+ contains=Special
syn region  semString 	start=+\[\[+ end=+\]\]+

" integers 
syn match   semNumber 	"\<[0-9]\+\>"

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_sem_syn_inits")
  if version < 508
    let did_sem_syn_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink semStatement Statement
  HiLink semRegister Operator
  HiLink semComment Comment
  HiLink semTodo Todo
  HiLink semConstant Special 
  HiLink semString String
  HiLink semNumber Number
  
  delcommand HiLink
endif

let b:current_syntax = "sem"
