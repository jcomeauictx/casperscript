%! bin/bccs
(testing...) =
/ARGUMENTS dup where {(ARGUMENTS: ) print exch get ==} {pop} ifelse
/abc (abc) def
abc =
%
(this will show, as ghostscript doesn't use readline for comments) =
% 
(def) = % this should print in any case
(final stack:) = pstack
