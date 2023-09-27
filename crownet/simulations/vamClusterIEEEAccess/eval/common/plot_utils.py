import os

def save_plot(fig, name):
    if not os.path.exists('fig'):
        os.makedirs('fig')

    fig.savefig(f"fig/{name}.png")    
    fig.savefig(f"fig/{name}.svg")
    fig.savefig(f"fig/{name}.pdf")
