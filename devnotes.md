## Git

```bash
git remote | xargs -I _ sh -c "echo pushing to _ ... && git push _ master"
```

## TODO

- minor improvements
   - copy(ArrView, ArrView) will not check if ranges intersects, this could cause problems
   - str replace method
