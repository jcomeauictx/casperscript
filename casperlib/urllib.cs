#!/usr/local/casperscript/bin/ccs --
/urllib 10 dict def urllib begin
  /parse 10 dict def parse begin
    /docstrings 10 dict def  % every "library" needs its own docstrings
    /quote ( str#unquoted [str#safe str#unsafe?] - str#quoted
      like Python3's `urllib.parse.quote`, escapes all characters
      that are neither in "unreserved" characters nor in `safe` arg
      but allows any range of safe and unsafe characters
      (removes from `unreserved` anything in `unsafe`)) docstring {
      10 dict begin  % for local variables
        /ALPHA [[latin1.A latin1.Z 1 add] range {chr} forall] () string.join
          [[latin1.a latin1.z 1 add] range {chr} forall] () string.join
          string.add def
        /DIGIT
          [[latin1.0 latin1.9 1 add] range {chr} forall] () string.join def
        /unreserved ALPHA DIGIT string.add (-._~) string.add def
        /gendelims (:/?#[]@) def
        /subdelims (!$&'()*+,;=) def
        /reserved gendelims subdelims string.add def
        % can't safely use `%` below without risk of it being scanned
        % as comment---using its octal escape \045 instead.
        /escape ( int#character - str#escaped
          escape the character, i.e. ( ) becomes (\04520)) docstring {
          (\045) exch 16 2 string cvrs string.add
        } bind def
      (stack before local defs: ) #only #stack
      dup /unsafe #stack exch {1 get} stopped {pop pop ()} if def
      /safe exch #stack {0 get} stopped {#stack pop pop ()} if def
      (safe: ) #only safe ##only (, unsafe: ) #only unsafe ##only
      (, unreserved: ) #only unreserved ##only
      (, reserved: ) #only reserved #
      % logic we're using:
      % REMOVE any `unsafe` characters from `unreserved`
      % ADD `safe` characters to `unreserved`
      % IF character in unreserved, pass it through unchanged
      % ELSE escape it
      unsafe {unreserved exch () -1 string.replace /unreserved exch def} forall
      (new unreserved: ) #only unreserved #
      unreserved safe string.add /unreserved exch def
      (new unreserved: ) #only unreserved #
      dup length 3 mul string exch % allow for entire string to be escaped
      {
        unreserved 1 index string.contains {chr} {escape} ifelse
        1 index exch string.append
      }
      forall
      (remaining stack: ) #only #stack
      string.truncate
      end  % local variables dict
    } bind def
  end  % urllib.parse
end  % urllib
scriptname (urllib) eq
  %urllib ###
  {
    {sys.argv 1 get} stopped {pop pop (this is a test)} if
    sys.argv [2] subarray urllib.parse.quote =
  }
  if
% vim: tabstop=8 shiftwidth=2 expandtab softtabstop=2 syntax=postscr
