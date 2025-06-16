
mono : tp4_top.desc tp4_top.cpp
	soclib-cc -P -p tp4_top.desc -o simulator.x

multi : tp4_top_multi.desc tp4_top_multi.cpp
	soclib-cc -m debug -P -p tp4_top_multi.desc -o simulator_multi.x

clean:
	rm -f *.o *.x core tmp *.deps stats trace simul*
	soclib-cc -x -X -P -p tp4_top_multi.desc  -o simulator_multi.x

