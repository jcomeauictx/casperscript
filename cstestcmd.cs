#! bin/gs -S -q -dNODISPLAY -dBATCH --
(testing...) =
/ARGUMENTS dup where {(ARGUMENTS: ) print exch get ==} {pop} ifelse
/abc (abc) def
abc =
(final stack:) = pstack
