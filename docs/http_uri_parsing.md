1. separate raw_uri into (scheme, authority, host, port,) path, query and fragment
2. ProcessPath():
   1. DecodePercentages()
   2. CheckForInvalidPathChars()
   3. Normalize():
      1. multiple "///" collapse to "/"
      2. "./" can be removed
      3. "../" deletes previous folder from uri (work with stack containing of strings that represent a folder each. if "../" leads to level that is above root -> ERROR (path traversing...)
3. ProcessQuery():
   1. DecodePercentages()
   2. CheckForInvalidQueryChars()
4. ProcessFragment()
   1. DecodePercentages()
   2. CheckForInvalidFramentChars()
