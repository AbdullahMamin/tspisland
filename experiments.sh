# A set of quick experiments on various test data. Outputs go to out/experiments
echo "== Running experiments... =="

population_size=8000
max_generations=5000
mutation_rate=0.05

tsplib_path="data/tsplib"
#problems=("eil51" "fl417" "lin105" "berlin52" "ch130" "ch150" "bier127" "p654" "d2103" "fl1400" "kroB200")
problems=("d2103")

cd ./out/bin
for problem in ${problems[@]}
do
    echo "> Doing" $problem
    ./tspisland --tsp_in=../../$tsplib_path/$problem.tsp --tour_out=../experiments/$problem.tour --population_size=$population_size --max_generations=$max_generations --mutation_rate=$mutation_rate --summary_out=../experiments/$problem.csv
    echo "> Done with" $problem
done

echo "> Generating graphs..."

cd ../experiments
for problem in ${problems[@]}
do
    python ../../scripts/tour_to_image.py ../../$tsplib_path/$problem.tsp $problem.tour $problem.svg
    python ../../scripts/summary_to_graphs.py $problem.csv $problem.summary.svg $problem.entropy.svg
done

echo "== Finished experiments! =="
