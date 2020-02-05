import os


class Cfg:

    default_cgf = {
        "general": {"INTNET": "rovernet"},
        "omnetpp": {
            "cmd": ["omnetpp"],
            "default_image_name": "omnetpp",
        },
        "vadere": {
            "cmd": ["/init.sh"],
            "default_image_name": "vadere",
        },
        "vadere-gui": {
            "cmd": ["java", "-jar", "/opt/vadere/vadere/VadereGui/target/vadere-gui.jar"],
            "default_image_name": "vadere"
        },
        "vadere-postvis": {
            "cmd": ["java", "-jar", "/opt/vadere/vadere/VadereGui/target/vadere-postvis.jar"],
            "default_image_name": "vadere",
        },
        "sumo": {
            "cmd": ["/init.sh"],
            "default_image_name": "sumo",
        },
        "sumo-gui": {
            "cmd": ["sumo-gui"],
            "default_image_name": "sumo",
        }
    }

    default_run_args = {
        "image": None,
        "name": None,
        "network": None,
        "remove": None,
        "user": None,
        "hostname": None,
        "detach": False,
        "tty": False,
        "stderr": True,
        "cap_add": ["SYS_PTRACE"],
        "working_dir": os.path.abspath(os.curdir),
        "environment": {"ROVER_MAIN": os.environ["ROVER_MAIN"], },
        "volumes": {
            f"{os.environ['HOME']}": {"bind": f"{os.environ['HOME']}", "mode": "rw"},
            "/etc/group": {"bind": "/etc/group", "mode": "ro"},
            "/etc/passwd": {"bind": "/etc/passwd", "mode": "ro"},
            "/etc/shadow": {"bind": "/etc/shadow", "mode": "ro"},
            "/etc/sudoers.d": {"bind": "/etc/sudoers.d", "mode": "ro"},
            "/tmp/.X11-unix": {"bind": "/tmp/.X11-unix", "mode": "rw"},
        },
    }

    @classmethod
    def run_args(cls):
        # todo load from filesystem and default to default_run_args
        return cls.default_run_args.copy()

    @classmethod
    def config(cls):
        # todo load from filesystem and default to default_cgf
        return cls.default_cgf.copy()

    @classmethod
    def get_attr(cls, cmd, key):
        return cls.config()[cmd][key]
