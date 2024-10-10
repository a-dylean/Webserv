for i in {1..2000}
do
  curl http://localhost:8082/cgi-bin/test.py > /dev/null 2> /dev/null &
  echo $i
done
