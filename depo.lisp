(project cc
  (kind lib)
  (files cc/*.cpp)
  (include 'pub cc/inc)
  (link 'pub 'sys 'win Shell32.lib)
)