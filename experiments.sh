# A set of quick experiments on various test data. Outputs go to out/experiments
echo "== Running experiments =="

n_procs=4

out_path="out/experiments"
tsplib_path="data/tsplib"
problem="berlin52"

binary=out/bin/tspisland
mkdir -p $out_path/$problem/population1000
mpirun -np $n_procs $binary examples/quick_ring_population1000.toml $tsplib_path/$problem.tsp $out_path/$problem/population1000

echo "== Finished experiments! =="
