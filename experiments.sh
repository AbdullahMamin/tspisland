# A quick experiment of the code. Modify as needed.
echo "== Running experiments =="

# ==== Change these =====
n_procs=4
problem="berlin52"
example=examples/quick_ring_population100.toml
# =======================

out_path="out/experiments"
tspdata_path="data/tsplib"

problem_file=$tspdata_path/$problem.tsp

binary="out/bin/tspisland"
scripts_dir="scripts"
graph_script=$scripts_dir/graphs.sh
mkdir -p $out_path/$problem/test_run
mpirun -np $n_procs $binary $example $problem_file $out_path/$problem/test_run
bash $graph_script $scripts_dir $problem_file $out_path/$problem/test_run

echo "== Finished experiments! =="
