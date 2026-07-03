# A set of quick experiments on various test data. Outputs go to out/experiments
echo "== Running experiments... =="

n_procs=6

tsplib_path="data/tsplib"
# problems=("eil51" "fl417" "lin105" "berlin52" "ch130" "ch150" "bier127" "p654" "d2103" "fl1400" "kroB200")
# problems=("d2103")
# problems=("berlin52")
problems=("bier127")

cd ./out/bin
for problem in ${problems[@]}
do
    echo "> Doing" $problem
    mpirun -np $n_procs tspisland --config_in=../../examples/genetic.toml --tsp_in=../../$tsplib_path/$problem.tsp --tour_out=../experiments/$problem.tour
    echo "> Done with" $problem
done

echo "> Generating graphs..."

cd ../experiments
for problem in ${problems[@]}
do
    echo "> Doing" $problem
    python ../../scripts/tour_to_image.py ../../$tsplib_path/$problem.tsp $problem.tour $problem.tour.svg
    # python ../../scripts/summary_to_graph.py $problem.summary.csv $problem.summary.svg
    # python ../../scripts/entropy_to_graph.py $problem.entropy.csv $problem.entropy.svg
    # python ../../scripts/heat_to_animation.py ../../$tsplib_path/$problem.tsp $problem.heat $problem.heat.mkv
    echo "> Done with" $problem
done

echo "== Finished experiments! =="
