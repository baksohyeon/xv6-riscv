#!/usr/bin/env python3
"""
Stride Scheduler Results Plotter
Generates graphs showing CPU distribution over time for stride scheduling test
"""

import matplotlib.pyplot as plt
import numpy as np
import sys
import re

def parse_xv6_output(filename):
    """Parse the output from testlottery and extract CSV data"""
    samples = []
    
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    # Find CSV section
    csv_start = -1
    csv_end = -1
    
    for i, line in enumerate(lines):
        if "=== CSV DATA FOR GRAPHING ===" in line:
            csv_start = i + 2  # Skip header line too
        elif "=== END CSV DATA ===" in line:
            csv_end = i
            break
    
    if csv_start == -1 or csv_end == -1:
        print("Error: Could not find CSV data section in output")
        return None
    
    # Parse CSV data
    for i in range(csv_start, csv_end):
        line = lines[i].strip()
        if line and not line.startswith('Time,'):  # Skip header
            parts = line.split(',')
            if len(parts) >= 7:
                try:
                    sample = {
                        'time': int(parts[0]),
                        'ticks_a': int(parts[1]),
                        'ticks_b': int(parts[2]),
                        'ticks_c': int(parts[3]),
                        'percent_a': int(parts[4]),
                        'percent_b': int(parts[5]),
                        'percent_c': int(parts[6])
                    }
                    samples.append(sample)
                except ValueError:
                    continue
    
    return samples

def create_graphs(samples, output_prefix="stride_scheduler"):
    """Create multiple graphs showing stride scheduler performance"""
    
    if not samples:
        print("No data to plot")
        return
    
    times = [s['time'] for s in samples]
    ticks_a = [s['ticks_a'] for s in samples]
    ticks_b = [s['ticks_b'] for s in samples]
    ticks_c = [s['ticks_c'] for s in samples]
    percent_a = [s['percent_a'] for s in samples]
    percent_b = [s['percent_b'] for s in samples]
    percent_c = [s['percent_c'] for s in samples]
    
    # Set up the plotting style
    plt.style.use('default')
    colors = ['#FF6B6B', '#4ECDC4', '#45B7D1']  # Red, Teal, Blue
    
    # Create figure with subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
    fig.suptitle('Stride Scheduler Performance Analysis\n(3:2:1 Ticket Ratio - 30:20:10)', 
                 fontsize=16, fontweight='bold')
    
    # 1. Absolute ticks over time
    ax1.plot(times, ticks_a, 'o-', color=colors[0], linewidth=2, markersize=6, label='Process A (30 tickets)')
    ax1.plot(times, ticks_b, 's-', color=colors[1], linewidth=2, markersize=6, label='Process B (20 tickets)')
    ax1.plot(times, ticks_c, '^-', color=colors[2], linewidth=2, markersize=6, label='Process C (10 tickets)')
    ax1.set_xlabel('Time Point')
    ax1.set_ylabel('Cumulative Ticks')
    ax1.set_title('Cumulative CPU Ticks Over Time')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # 2. CPU percentage over time
    ax2.plot(times, percent_a, 'o-', color=colors[0], linewidth=2, markersize=6, label='Process A (Expected: 50%)')
    ax2.plot(times, percent_b, 's-', color=colors[1], linewidth=2, markersize=6, label='Process B (Expected: 33%)')
    ax2.plot(times, percent_c, '^-', color=colors[2], linewidth=2, markersize=6, label='Process C (Expected: 17%)')
    
    # Add expected ratio lines
    ax2.axhline(y=50, color=colors[0], linestyle='--', alpha=0.5)
    ax2.axhline(y=33, color=colors[1], linestyle='--', alpha=0.5)
    ax2.axhline(y=17, color=colors[2], linestyle='--', alpha=0.5)
    
    ax2.set_xlabel('Time Point')
    ax2.set_ylabel('CPU Percentage (%)')
    ax2.set_title('CPU Distribution Over Time')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(0, 100)
    
    # 3. Stacked area chart
    ax3.stackplot(times, percent_a, percent_b, percent_c, 
                  colors=colors, alpha=0.7, 
                  labels=['Process A (30 tickets)', 'Process B (20 tickets)', 'Process C (10 tickets)'])
    ax3.set_xlabel('Time Point')
    ax3.set_ylabel('CPU Percentage (%)')
    ax3.set_title('CPU Distribution (Stacked)')
    ax3.legend(loc='upper right')
    ax3.grid(True, alpha=0.3)
    ax3.set_ylim(0, 100)
    
    # 4. Final distribution bar chart
    if samples:
        final_sample = samples[-1]
        processes = ['Process A\n(30 tickets)', 'Process B\n(20 tickets)', 'Process C\n(10 tickets)']
        actual_percent = [final_sample['percent_a'], final_sample['percent_b'], final_sample['percent_c']]
        expected_percent = [50, 33, 17]
        
        x = np.arange(len(processes))
        width = 0.35
        
        bars1 = ax4.bar(x - width/2, actual_percent, width, label='Actual', color=colors, alpha=0.8)
        bars2 = ax4.bar(x + width/2, expected_percent, width, label='Expected', 
                       color=colors, alpha=0.4, edgecolor='black', linewidth=1)
        
        ax4.set_xlabel('Processes')
        ax4.set_ylabel('CPU Percentage (%)')
        ax4.set_title('Final CPU Distribution Comparison')
        ax4.set_xticks(x)
        ax4.set_xticklabels(processes)
        ax4.legend()
        ax4.grid(True, alpha=0.3, axis='y')
        
        # Add value labels on bars
        for bar in bars1:
            height = bar.get_height()
            ax4.text(bar.get_x() + bar.get_width()/2., height + 0.5,
                    f'{height}%', ha='center', va='bottom', fontweight='bold')
        
        for bar in bars2:
            height = bar.get_height()
            ax4.text(bar.get_x() + bar.get_width()/2., height + 0.5,
                    f'{height}%', ha='center', va='bottom', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_analysis.png', dpi=300, bbox_inches='tight')
    print(f"Graph saved as {output_prefix}_analysis.png")
    
    # Create a simplified single graph for the assignment
    plt.figure(figsize=(12, 8))
    plt.plot(times, ticks_a, 'o-', color=colors[0], linewidth=3, markersize=8, label='Process A (30 tickets, Expected: 50%)')
    plt.plot(times, ticks_b, 's-', color=colors[1], linewidth=3, markersize=8, label='Process B (20 tickets, Expected: 33%)')
    plt.plot(times, ticks_c, '^-', color=colors[2], linewidth=3, markersize=8, label='Process C (10 tickets, Expected: 17%)')
    
    plt.xlabel('Time Point', fontsize=14)
    plt.ylabel('Cumulative CPU Ticks', fontsize=14)
    plt.title('Stride Scheduler Performance: Time Slices Over Time\n(3:2:1 Ticket Ratio)', fontsize=16, fontweight='bold')
    plt.legend(fontsize=12)
    plt.grid(True, alpha=0.3)
    
    # Add annotations for final values
    if samples:
        final_sample = samples[-1]
        final_time = times[-1]
        plt.annotate(f'Final: {final_sample["ticks_a"]} ticks', 
                    xy=(final_time, ticks_a[-1]), xytext=(10, 10), 
                    textcoords='offset points', fontsize=10, color=colors[0])
        plt.annotate(f'Final: {final_sample["ticks_b"]} ticks', 
                    xy=(final_time, ticks_b[-1]), xytext=(10, 10), 
                    textcoords='offset points', fontsize=10, color=colors[1])
        plt.annotate(f'Final: {final_sample["ticks_c"]} ticks', 
                    xy=(final_time, ticks_c[-1]), xytext=(10, 10), 
                    textcoords='offset points', fontsize=10, color=colors[2])
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_simple.png', dpi=300, bbox_inches='tight')
    print(f"Simple graph saved as {output_prefix}_simple.png")
    
    # Print summary statistics
    print("\n=== STRIDE SCHEDULER ANALYSIS ===")
    if samples:
        final = samples[-1]
        print(f"Final CPU Distribution:")
        print(f"  Process A: {final['percent_a']}% (Expected: 50%)")
        print(f"  Process B: {final['percent_b']}% (Expected: 33%)")
        print(f"  Process C: {final['percent_c']}% (Expected: 17%)")
        
        # Calculate average percentages
        avg_a = sum(percent_a) / len(percent_a)
        avg_b = sum(percent_b) / len(percent_b)
        avg_c = sum(percent_c) / len(percent_c)
        
        print(f"\nAverage CPU Distribution:")
        print(f"  Process A: {avg_a:.1f}% (Expected: 50%)")
        print(f"  Process B: {avg_b:.1f}% (Expected: 33%)")
        print(f"  Process C: {avg_c:.1f}% (Expected: 17%)")
        
        # Calculate fairness
        error_a = abs(avg_a - 50)
        error_b = abs(avg_b - 33)
        error_c = abs(avg_c - 17)
        total_error = error_a + error_b + error_c
        
        print(f"\nFairness Analysis:")
        print(f"  Average deviation from expected: {total_error/3:.1f}%")
        print(f"  Scheduler efficiency: {100 - (total_error/3):.1f}%")

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 plot_scheduler_results.py <xv6_output_file>")
        print("Example: python3 plot_scheduler_results.py testlottery_output.txt")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    try:
        samples = parse_xv6_output(filename)
        if samples:
            create_graphs(samples)
        else:
            print("No valid data found in the output file")
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
