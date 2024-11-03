#! bin/bccs
(testing...) =
/ARGUMENTS dup where {(ARGUMENTS: ) print exch get ==} {pop} ifelse
/abc (abc) def
abc =
(def) =
(final stack:) = pstack
