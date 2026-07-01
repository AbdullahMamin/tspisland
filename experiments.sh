# A set of quick experiments on various test data. Outputs go to out/experiments
echo "== Running experiments... =="

n_procs=6

population_size=100
max_generations=100
mutation_rate=0.01
max_mutation_strength=0.05

tsplib_path="data/tsplib"
# problems=("eil51" "fl417" "lin105" "berlin52" "ch130" "ch150" "bier127" "p654" "d2103" "fl1400" "kroB200")
# problems=("d2103")
problems=("berlin52")
# problems=("lin105")

cd ./out/bin
for problem in ${problems[@]}
do
    echo "> Doing" $problem
    mpirun -np $n_procs tspisland --tsp_in=../../$tsplib_path/$problem.tsp --tour_out=../experiments/$problem.tour --population_size=$population_size --max_generations=$max_generations --mutation_rate=$mutation_rate --max_mutation_strength=$max_mutation_strength --summary_out=../experiments/$problem.summary.csv --entropy_out=../experiments/$problem.entropy.csv --heat_out=../experiments/$problem.heat
    echo "> Done with" $problem
done

echo "> Generating graphs..."

cd ../experiments
for problem in ${problems[@]}
do
    python ../../scripts/tour_to_image.py ../../$tsplib_path/$problem.tsp $problem.tour $problem.tour.svg
    python ../../scripts/summary_to_graph.py $problem.summary.csv $problem.summary.svg
    python ../../scripts/entropy_to_graph.py $problem.entropy.csv $problem.entropy.svg
    python ../../scripts/heat_to_animation.py ../../$tsplib_path/$problem.tsp $problem.heat $problem.heat.mkv
done

echo "== Finished experiments! =="
