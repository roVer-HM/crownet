import os

def save_plot(fig, name):
    if not os.path.exists('fig'):
        os.makedirs('fig')

    fig.savefig(f"fig/{name}.png", bbox_inches='tight')    
    fig.savefig(f"fig/{name}.svg", bbox_inches='tight')
    fig.savefig(f"fig/{name}.pdf", bbox_inches='tight')
