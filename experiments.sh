# A set of quick experiments on various test data. Outputs go to out/experiments
echo "== Running experiments =="

n_procs=5

out_path="out/experiments"
tsplib_path="data/tsplib"
# problems=("eil51" "fl417" "lin105" "berlin52" "ch130" "ch150" "bier127" "p654" "d2103" "fl1400" "kroB200")
# problems=("d2103")
# problems=("berlin52")
problem="berlin52"

binary=out/bin/tspisland
mkdir -p $out_path/$problem
mpirun -np $n_procs $binary examples/ring_topology.toml $tsplib_path/$problem.tsp $out_path/$problem

python3 scripts/summary_to_graph.py $out_path/$problem/island_0.summary.csv $out_path/$problem/island_0.summary.svg
python3 scripts/summary_to_graph.py $out_path/$problem/island_1.summary.csv $out_path/$problem/island_1.summary.svg
python3 scripts/summary_to_graph.py $out_path/$problem/island_2.summary.csv $out_path/$problem/island_2.summary.svg
python3 scripts/tour_to_image.py $tsplib_path/$problem.tsp $out_path/$problem/island_0.tour $out_path/$problem/island_0.tour.svg
python3 scripts/tour_to_image.py $tsplib_path/$problem.tsp $out_path/$problem/island_1.tour $out_path/$problem/island_1.tour.svg
python3 scripts/tour_to_image.py $tsplib_path/$problem.tsp $out_path/$problem/island_2.tour $out_path/$problem/island_2.tour.svg
python3 scripts/profile_to_entropy.py $out_path/$problem/island_0.profile $out_path/$problem/island_0.entropy.svg
python3 scripts/profile_to_entropy.py $out_path/$problem/island_1.profile $out_path/$problem/island_1.entropy.svg
python3 scripts/profile_to_entropy.py $out_path/$problem/island_2.profile $out_path/$problem/island_2.entropy.svg

echo "== Finished experiments! =="
