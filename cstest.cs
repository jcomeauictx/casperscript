#! bin/cs -I_DEV_ --
(testing...) =
/ARGUMENTS dup where {(ARGUMENTS: ) print exch get ==} {pop} ifelse
/abc (abc) def
10 10 moveto
/Helvetica 20 selectfont
abc show
showpage
