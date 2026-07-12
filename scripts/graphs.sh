scripts_dir=$1
problem=$2
data_dir=$3

summary_script=$scripts_dir/summary_to_graph.py
entropy_script=$scripts_dir/entropy_to_graph.py
tour_script=$scripts_dir/tour_to_image.py

echo === Generating graphs ===

for f in $data_dir/*.tour; do
    echo Turning $f into a graph
    python3 $tour_script $problem $f $f.svg
done

for f in $data_dir/*.entropy.csv; do
    echo Turning $f into a graph
    python3 $entropy_script $f ${f%.csv}.svg
done

for f in $data_dir/*.summary.csv; do
    echo Turning $f into a graph
    python3 $summary_script $f ${f%.csv}.svg
done

echo === Done! ===
