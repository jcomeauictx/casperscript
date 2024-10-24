#! bin/cs -S --
% adapted from https://stackoverflow.com/a/15167678/493161
/inch {72 mul} def

/ocimage
  <<
    /ImageType 1
    /Width 5 inch
    /Height 7 inch
    /ImageMatrix [5 inch 0 0 7 inch neg 0 7 inch]
    /MultipleDataSources false
    /DataSource (/dev/urandom) (r) file
    /BitsPerComponent 8
    /Decode [0 1 0 1 0 1]
    /Interpolate false
  >> def

save
1 inch 2 inch moveto currentpoint translate
5 inch 7 inch scale
ocimage
/DeviceRGB setcolorspace image
restore
showpage

save
1 inch 2 inch moveto currentpoint translate
5 inch 7 inch scale
ocimage 
dup /Width get exch
dup /Height get exch
dup /BitsPerComponent get exch
dup /ImageMatrix get exch
dup /DataSource get exch
dup /MultipleDataSources 2 copy known {get} {pop pop false} ifelse exch
/Decode get length 2 div cvi % number of colors
/DeviceRGB setcolorspace colorimage
restore
showpage

(imagedict: ) = ocimage ===  % display on stdout
