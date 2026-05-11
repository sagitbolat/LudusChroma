# Define the directory to search for source code files
directory="./"

# Define the directories to ignore (comma-separated list)
ignore_directories=("/Engine/UI/imgui" "/Engine/UI/nuklear" "/Engine/stb" "/Engine/assets" "build")

# Define the file extensions to consider (e.g., .py, .js, .java)
extensions=("h" "cpp" "c" "hpp")

# Initialize the line count variable
line_count=0

# Construct the ignore directories argument for find command
ignore_arg=""
for dir in "${ignore_directories[@]}"; do
    ignore_arg+=" -not -path \"*/$dir/*\""
done

# Loop through the files with the specified extensions
for ext in "${extensions[@]}"; do
    count=$(find "$directory" $ignore_arg -type f -name "*.$ext" -exec grep -v '^$' {} \; | wc -l)
    line_count=$((line_count + count))
done

# Display the total line count
echo "Total lines of code: $line_count"
