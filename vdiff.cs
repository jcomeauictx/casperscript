#! bin/cs --
% visual diff two pnm files

/inch {72 mul} bind def
/zero (0) 0 get def  % value of ASCII zero (0x30)

/bits <<  % translate maxvalue into bits per sample
  1 1
  255 8
>> def

/decode <<  % Pn type into Decode matrix
  /P1 [0 1]
  /P2 [0 1]
  /P3 [0 1 0 1 0 1]
>> def

/binarize <<  % Pn type into binary stream
  /P1 {
    % read until (0), (1), or EOF
    {infile read
      {dup 1 not and zero eq
        {zero - 1 string dup 0 4 -1 roll put exit}
        {pop}
        ifelse}
      {() exit}
      ifelse
    } loop
  }
  /P2 {infile token {1 string dup 0 4 -1 roll put} {()} ifelse}
  /P3 {infile token {1 string dup 0 4 -1 roll put} {()} ifelse}
>> def

/colorspaces <<  % Pn type into colorspace
  /P1 /DeviceGray
  /P2 /DeviceGray
  /P3 /DeviceRGB
>> def

/pnm <<  % valid type 1 image dict once fleshed out
  /ImageType 1
  /Height 0
  /Width 0
  /ImageMatrix [/w 0 0 /-h 0 /h]  % placeholder for top-bottom left-right data
  /MultipleDataSources false  % optional (default) but let's be explicit
  /DataSource null  % replace with input stream
  /BitsPerComponent 1  % default for P1
  /Decode [0 1]  % map colors into range 0 to 1 ([0 1 0 1 0 1] for RGB)
  /Interpolate false  % another optional, default false
>> def

/comment (#) 0 get def  % comment introducer

/readpnm {  % filename - pnminstance colorspace
  % read PNM file into dict
  % we confine ourselves to gs-created pnm files, which only have a single
  % whole-line comment on 2nd line. this won't work with end-of-line comments.
  pnm dup length dict copy /instance exch def  % new instance of pnm dictionary
  (r) file /infile exch def  % store open file handle in `infile`.
  /buffer 1024 string def  % hopefully enough for any PNM line length
  /pnmtype infile token pop cvlit def  % /P1, /P2, or /P3
  instance /Decode decode pnmtype get put
  /colorspace colorspaces pnmtype get def
  % read and discard the expected comment, throw error on failure.
  infile buffer readline pop 0 get comment ne {
    (Not gs-created pnm file) = /valueerror signalerror
  } if
  instance /Width infile token pop cvi dup /width exch def put
  instance /Height infile token pop cvi dup /height exch def put
  instance /ImageMatrix get dup dup  % we're going to overwrite the /w etc.
    0 width put
    3 height neg put
    5 height put
  pnmtype (P1) ne {  % P2 and P3 have an extra line, max value
    instance /BitsPerComponent bits infile token pop cvi get put
  } if
  instance /DataSource binarize pnmtype get /ReusableStreamDecode filter put
  instance dup (instance: ) print === colorspace
} bind def

/vdiff {  % filename1 filename2 sidebyside -
  % generate pnm of differences between two pnm files
  % (vdiff called, stack:) = pstack
  /sidebyside exch def
  /filename2 exch def
  /filename1 exch def
  filename1 readpnm
  gsave
  setcolorspace
  dup /Width get /width exch def
  dup /Height get /height exch def
  0 0 moveto
  (drawing image 1 at ) print currentpoint exch 10 string cvs print ( ) print =
  currentpoint translate width height scale dup
  image
  grestore
  sidebyside not {showpage} if  % only show if *not* sidebyside mode
  /DataSource get dup dup resetfile bytesavailable (data length: ) print =
  (pstack after first file, should have data: ) = pstack
  filename2 readpnm
  gsave
  dup setcolorspace exch
  (pstack after 2nd setcolorspace, should be dict color data: ) = pstack
  sidebyside {width} {0} ifelse 0 moveto
  (drawing image 2 at ) print currentpoint exch 10 string cvs print ( ) print =
  currentpoint translate width height scale dup dup
  image
  grestore
  /DataSource get dup dup resetfile bytesavailable (data length: ) print =
  showpage
  (pstack after showpage, should have data dict color data: ) = pstack
  3 -1 roll  % put colorspace on top
  gsave
  dup /colorspace exch def setcolorspace
  (pstack after setcolorspace, should have file2data dict file1data: ) = pstack
  0 0 moveto
  (drawing image 3 at ) print currentpoint exch 10 string cvs print ( ) print =
  currentpoint translate
  3 -1 roll  % put file1data on top
  /file1data exch def /file2data exch def
  (pstack after data removed: ) = pstack
  dup /BitsPerComponent get 1 exch bitshift 1 sub /maxvalue exch def
  dup  % extra copy of dict for `image` operator
  /DataSource {file1data read file2data read  % n true n true, or false false
    {exch pop sub abs maxvalue exch sub 1 string dup 3 -1 roll 0 exch put}
    {pop ()} ifelse
  } put
  (pstack after setting DataSource: ) = pstack
  (pstack before scale: ) = pstack
  width height scale dup 
  image
  grestore
  sidebyside not {showpage} if  % only show here if *not* sidebyside mode
  (pstack after "positive" diff: ) = pstack
  dup  % extra copy of dict for `image` operator
  % rewind data files before reusing for "negative" diff
  file1data dup resetfile bytesavailable (data length: ) print =
  file2data resetfile
  /DataSource {file1data read file2data read  % n true n true, or false false
    {exch pop sub abs 1 string dup 3 -1 roll 0 exch put}  % "negative" diff
    {pop ()} ifelse
  } put
  (pstack after setting DataSource: ) = pstack
  (pstack before final scale: ) = pstack
  gsave
  colorspace setcolorspace
  sidebyside {width} {0} ifelse 0 moveto
  (drawing image 4 at ) print currentpoint exch 10 string cvs print ( ) print =
  currentpoint translate
  width height scale
  (pstack before final image: ) = pstack
  (image dict: ) = dup ===
  image
  grestore
  showpage
  (pstack at end: ) = pstack
} bind def

scriptname (vdiff) eq {
  sys.argv dup 1 get exch dup 2 get exch {3 get} stopped
  {pop pop false} {cvi 0 gt} ifelse  % sidebyside mode if > 0
  vdiff
} if
% vim: tabstop=8 shiftwidth=2 expandtab softtabstop=2
