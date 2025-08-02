#!/bin/bash

# Stride Scheduler Test Runner
# This script runs the stride scheduler test and generates graphs

echo "=== Stride Scheduler Test Runner ==="
echo "Building xv6 with stride scheduler..."

# Build xv6
make clean
if ! make; then
    echo "Error: Failed to build xv6"
    exit 1
fi

echo ""
echo "Starting xv6 and running stride scheduler test..."
echo "This will take approximately 2-3 minutes to complete."
echo ""

# Create output directory
mkdir -p test_results

# Run the test and capture output
echo "Running testlottery..."
echo "Note: You need to manually run 'testlottery' in the xv6 shell"
echo "The test output should be saved to a file for graph generation."

# Instructions for manual execution
echo ""
echo "=== MANUAL EXECUTION INSTRUCTIONS ==="
echo "1. Run: make qemu"
echo "2. In xv6 shell, run: testlottery"
echo "3. Copy the output to a file named 'test_results/stride_output.txt'"
echo "4. Run: python3 plot_scheduler_results.py test_results/stride_output.txt"
echo ""
echo "Alternative: Run with output redirection:"
echo "1. Run: make qemu-nox > test_results/stride_output.txt 2>&1"
echo "2. In xv6 shell, run: testlottery"
echo "3. Exit xv6: press Ctrl+A then X"
echo "4. Run: python3 plot_scheduler_results.py test_results/stride_output.txt"

# Check if Python and matplotlib are available
echo ""
echo "Checking Python dependencies..."
if command -v python3 &> /dev/null; then
    echo "✓ Python3 is available"
    if python3 -c "import matplotlib" 2>/dev/null; then
        echo "✓ matplotlib is available"
    else
        echo "⚠ matplotlib not found. Install with: pip3 install matplotlib"
    fi
    if python3 -c "import numpy" 2>/dev/null; then
        echo "✓ numpy is available"
    else
        echo "⚠ numpy not found. Install with: pip3 install numpy"
    fi
else
    echo "⚠ Python3 not found. Please install Python3"
fi

echo ""
echo "=== EXPECTED RESULTS ==="
echo "The stride scheduler should show:"
echo "- Process A (30 tickets): ~50% CPU time"
echo "- Process B (20 tickets): ~33% CPU time" 
echo "- Process C (10 tickets): ~17% CPU time"
echo ""
echo "Graphs will be saved as:"
echo "- stride_scheduler_analysis.png (detailed analysis)"
echo "- stride_scheduler_simple.png (simple time series)"
