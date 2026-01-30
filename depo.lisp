(project cc
  (kind lib)
  (files cc/*.cpp cc/*.c)
  (include 'pub cc/inc)
  (include cc/src)
  (link 'pub 'sys 'win Shell32.lib)
)

(project cc-tests
  (kind exe)
  (files cc-tests/*.cpp cc-tests/*.c)
  (link 'prj cc)
  (include cc-tests/inc cc-tests/src)
)

(targets cc-tests)
