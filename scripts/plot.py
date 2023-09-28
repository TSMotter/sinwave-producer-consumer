import json
import logging
import os
import argparse
import matplotlib.pyplot as plt


""" Example of usage: python scripts/plot.py -f output.log -s """

"""**********
    MAIN
**********"""
if __name__ == '__main__':
    try:
        parser = argparse.ArgumentParser(
            description='Plot points that are saved in a file in json format',
            epilog='No epilog')
        parser.add_argument(
            '-f',
            '--file',
            type=str,
            required=True,
            help="Path to input file"),
        parser.add_argument('-s', '--save',
            default=False,
            action="store_true",
            help="Save the plotted graph into a file")
        parser.add_argument(
            '-l',
            '--loglevel',
            default="INFO",
            type=str,
            required=False,
            help="Choose one of: ERROR, WARNING, INFO, DEBUG"),
        parser.add_argument('--dryrun', default=False, action='store_true')
        args = parser.parse_args()
        logging.basicConfig(level=args.loglevel)

        # File path
        file_path = args.file

        # Lists to store x and y values
        x_values = []
        y_values = []

        # Read file line by line
        with open(file_path, "r") as file:
            lines = file.readlines()
            for line in lines[:-1]:
                # Parse JSON from each line
                data = json.loads(line)
                
                # Extract channel, time, and value from the JSON data
                channel = data["channel"]
                time = data["time"]
                value = data["value"]
                
                # Append to the x and y value lists
                x_values.append(time)
                y_values.append(value)

        # Plot the points
        plt.scatter(x_values, y_values, label="Points")

        # Plot the translucent line
        plt.plot(x_values, y_values, color="black", alpha=0.5, label="Line")

        plt.xlabel("Time[seconds]")
        plt.ylabel("Value[no dimension]")
        plt.title(channel)  # Use channel as the plot's title
        plt.grid(True)  # Add a grid
        plt.legend()  # Show legend with labels
        if(args.save):
            plt.savefig("graph.png")  # Save the plotted graph as an image
        plt.show()

    except KeyboardInterrupt:
        logging.info('KeyboardInterrupt caught. Performing any cleanup needed')
        exit(0)