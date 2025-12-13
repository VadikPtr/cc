(project cc
  (kind lib)
  (files cc/*.cpp cc/*.c)
  (include 'pub cc/inc)
  (include cc/src)
  (link 'pub 'sys 'win Shell32.lib)
)