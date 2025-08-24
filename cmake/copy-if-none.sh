#echo "TODO (COPY): $@"

echo "result dir: $1"
dstdir=$1
mkdir -p $dstdir

shift

for f in $@
do
  fn=$(basename -- "$f")
  dst="$dstdir/$fn"
  if ! test -f $dst
  then
    echo "COPY: $f TO"
    echo "      $dst"
    cp $f $dst
  fi
done
