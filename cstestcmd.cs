#!/usr/bin/env -S /usr/src/jcomeauictx/casperscript/bin/cs -q -dNODISPLAY -dBATCH --
(testing...) =
/ARGUMENTS dup where {(ARGUMENTS: ) print exch get ==} {pop} ifelse
/abc (abc) def
abc =
