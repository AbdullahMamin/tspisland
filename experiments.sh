# A quick experiment of the code. Modify as needed.
echo "== Running experiments =="

n_procs=4

out_path="out/experiments"
tspdata_path="data/tsplib"
problem="berlin52"

binary=out/bin/tspisland
mkdir -p $out_path/$problem/test_run
mpirun -np $n_procs $binary examples/quick_ring_population100.toml $tspdata_path/$problem.tsp $out_path/$problem/test_run

echo "== Finished experiments! =="
