#! bin/cs -S -C --
% visual diff two pnm files

/inch {72 mul} bind def

/bits <<  % translate maxvalue into bits per sample
  1 1
  255 8
>> def

/decode <<  % Pn type into Decode matrix
  /P1 [0 1]
  /P2 [0 1]
  /P3 [0 1 0 1 0 1]
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
  instance /DataSource {
    infile token {1 string dup 0 4 -1 roll put} {()} ifelse
  } /ReusableStreamDecode filter put
  instance dup (instance: ) print === colorspace
} bind def

/vdiff {  % filename1 filename2 sidebyside -
  % generate pnm of differences between two pnm files
  % (vdiff called, stack:) = pstack
  /sidebyside exch def
  /filename2 exch def
  /filename1 exch def
  filename1 .25 inch .25 inch translate readpnm
  setcolorspace
  dup /Width get exch dup /Height get exch 3 1 roll scale dup image
  /DataSource get dup dup resetfile bytesavailable (data length: ) print =
  showpage
  (pstack after first file, should have data: ) = pstack
  filename2 .25 inch .25 inch translate readpnm
  setcolorspace
  dup dup /Width get exch dup /Height get exch 3 1 roll scale dup image
  /DataSource get dup dup resetfile bytesavailable (data length: ) print =
  showpage
  (pstack after second file, should have data, dict, data: ) = pstack
  (pstack at end: ) = pstack
} bind def

argv dup 1 get exch dup 2 get exch {3 get} stopped {pop pop false} if vdiff