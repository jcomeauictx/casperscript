#! bin/bccs
(testing...) =
/ARGUMENTS dup where {(ARGUMENTS: ) print exch get ==} {pop} ifelse
/abc (abc) def
abc =
#
(this should not show, as the preceding line has no following space) =
# 
(def) = # this should print, since the preceding line has following space
(final stack:) = pstack
