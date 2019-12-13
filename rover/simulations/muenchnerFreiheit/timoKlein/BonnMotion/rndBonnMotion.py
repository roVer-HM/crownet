import os
import random
import argparse
import sys


def main(arg_dict):
    if arg_dict['seed'] is None:
        seed = random.randrange(sys.maxsize)
    else:
        seed = arg_dict['seed']

    rnd = random.Random(seed)

    if not os.path.isfile(arg_dict['input']):
        print(f"input {arg_dict['input']} is not a file ")
        sys.exit(-1)

    with open(arg_dict['input'], 'r') as f:
        lines = f.readlines()

    draw = set()
    while len(draw) < arg_dict['n']:
        draw.add(rnd.randint(0, len(lines) - 1))

    with open(arg_dict['output'], 'w') as f:
        # if arg_dict['log-settings']:
        #     f.write(f"# Args: {arg_dict}. With seed={seed}\n")
        for d in draw:
            f.write(lines[d])

    print(f"done.")

def str2bool(v):
    # see https://stackoverflow.com/a/43357954
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


def parse_args(args):
    parser = argparse.ArgumentParser(prog="BonnMotion Helper",
                                     description="Crate subset of trajectories from a given BonnMotion file")

    parser.add_argument("-i", "--input",
                        dest="input",
                        nargs="?",
                        required=True,
                        help="BonnMotion input file")

    parser.add_argument("-o", "--output",
                        dest="output",
                        nargs="?",
                        required=True,
                        help="Location of Output file")

    parser.add_argument("-s", "--seed",
                        dest="seed",
                        required=False,
                        type=int,
                        help="seed for Randomness. Set this to some long value to reproduce outputs.")

    parser.add_argument("-n",
                        dest="n",
                        required=True,
                        type=int,
                        nargs="?",
                        help="number of samples to draw. If bigger than input only the input is returned.")

    parser.add_argument("--log-settings",
                        dest="log-settings",
                        type=str2bool,
                        const=True,
                        default=True,
                        nargs="?",
                        help="add command info to output file.")

    return parser.parse_args(args)


if __name__ == '__main__':
    # main(vars(parse_args(['--log-settings', '-i', 'mf_timo_klein_initial_peds/bn_0000.txt', '-o', 'out.txt', '-n', '3'])))
    main(vars(parse_args(sys.argv[1:])))
