#!/bin/sh -e

ODIN_pytest3conf=$1;shift;
ODIN_FILE=$1;       shift; 
ODIN_env=$1;        shift;
ODIN_stderr=$1;     shift;
ODIN_dir=$1;        shift;
pySp="$1";          shift;
ODIN_extra_env=$1;  shift;
ODIN_nocov=$1;      shift;
runcov="$1";        shift

if [ -z "$pySp" ]
then
    pySp=/dev/null
fi
if [ -z "$ODIN_stderr" ]
then
    ODIN_stderr="output"
fi
PYPATH=$(
    echo -n "$ODIN_DIR" &&
    cat "$pySp"|
	while read n
	do
	    echo -n ":$n"
	done
      )

pytest3=$ODIN_PYTEST3

if [ -z "$ODIN_pytest3conf" ] ; then
  ODIN_pytest3conf=$ODIN_PYTEST3_CONF;
fi

if [ -n "$ODIN_extra_env" ] ; then
  ODIN_extra_env=$(cat "$ODIN_extra_env" |
    while read n ; do echo -n " $n" ; done )
fi

if [ -z "$nocov" ]
then
  cov="$runcov ../pytest.py3.cov "
else
  cov=""
fi

first() {
  echo $1
}
rest() {
  shift
  echo "$@"
}

pytest3_exe=$(PATH="$ODIN_EXEC_PATH" which $(first $pytest3) ) || {
  echo "ERROR: $(first $pytest3) (first word of \$ODIN_PYTEST3 specified as $pytest3 when odin cache \$ODIN was created) not found on path $ODIN_EXEC_PATH failed, noting "
  false
}
pytest3="$pytest3_exe $(rest $pytest3)"

pytest_overrides=" -o cache_dir=$(pwd)/pytest3.exec/pytest-cache"

L=""
if [ "$ODINVERBOSE" != "" ] ; then
   echo ${ODINRBSHOST}env - LD_LIBRARY_PATH="$ODIN_EXEC_LD_LIBRARY_PATH" PATH="$ODIN_EXEC_PATH" PYTHONPATH="$PYPATH:$ODIN_PYTHON3PATH" \`cat "$ODIN_env"\` PYTEST_PLUGINS="$ODIN_PYTEST3_PLUGINS" PYTHONUNBUFFERED=1 $ODIN_extra_env $pytest3 -c $ODIN_pytest3conf $pytest_overrides "$ODIN_FILE"; 
fi
(
  mkdir pytest3.exec
  cd pytest3.exec
  env - LD_LIBRARY_PATH="$ODIN_EXEC_LD_LIBRARY_PATH" PATH="$ODIN_EXEC_PATH" PYTHONPATH="$PYPATH:$ODIN_PYTHONPATH" python3 -c "import pickle; pickle.dump(({},{},{}),open('pytest.py3.cov','wb'))"
  mkdir files
  touch errors
  touch output
  (
    cd files
    if [ $ODIN_stderr = "trace" ]
    then
      eval env - LD_LIBRARY_PATH="$ODIN_EXEC_LD_LIBRARY_PATH" PATH="$ODIN_EXEC_PATH" PYTHONPATH="$PYPATH:$ODIN_PYTHON3PATH" `cat "$ODIN_env"`  PYTEST_PLUGINS="$ODIN_PYTEST3_PLUGINS" PYTHONUNBUFFERED=1 $ODIN_extra_env python3 $cov $pytest3 -c $ODIN_pytest3conf $pytest_overrides "$ODIN_FILE" 2>&1 >../output
      echo $? > ../status
    elif  [ $ODIN_stderr = "output" ]
    then
      eval env - LD_LIBRARY_PATH="$ODIN_EXEC_LD_LIBRARY_PATH" PATH="$ODIN_EXEC_PATH" PYTHONPATH="$PYPATH:$ODIN_PYTHON3PATH" `cat "$ODIN_env"` PYTEST_PLUGINS="$ODIN_PYTEST3_PLUGINS" PYTHONUNBUFFERED=1 $ODIN_extra_env python3 $cov $pytest3 -c $ODIN_pytest3conf $pytest_overrides "$ODIN_FILE" >../output 2>&1
      echo $? > ../status
    elif  [ $ODIN_stderr = "error" ]
    then
      eval env - LD_LIBRARY_PATH="$ODIN_EXEC_LD_LIBRARY_PATH" PATH="$ODIN_EXEC_PATH" PYTHONPATH="$PYPATH:$ODIN_PYTHON3PATH" `cat "$ODIN_env"` PYTEST_PLUGINS="$ODIN_PYTEST3_PLUGINS" PYTHONUNBUFFERED=1 $ODIN_extra_env python3 $cov $pytest3 -c $ODIN_pytest3conf $pytest_overrides "$ODIN_FILE" >../output 2>../errors
      echo $? > ../status
    elif [ $ODIN_stderr = "warn" ]
    then
      eval env - LD_LIBRARY_PATH="$ODIN_EXEC_LD_LIBRARY_PATH" PATH="$ODIN_EXEC_PATH" PYTHONPATH="$PYPATH:$ODIN_PYTHON3PATH" `cat "$ODIN_env"` PYTEST_PLUGINS="$ODIN_PYTEST3_PLUGINS" PYTHONUNBUFFERED=1 $ODIN_extra_env python3 $cov $pytest3 -c $ODIN_pytest3conf $pytest_overrides "$ODIN_FILE" >../output
      echo $? > ../status
    else
      echo "error: +stderr, \"$ODIN_stderr\" is not one of trace, output, error, warn.">&2
      false
    fi
  )
  if [ -z "`ls files`" ]
  then
    ( cd files && tar cf - . )
  else
    ( cd files && tar cf - * )
  fi > files.tar
  rm -rf files pytest-cache

) <$ODIN_FILE 2>WARNINGS ||
mv WARNINGS ERRORS

exit 0
