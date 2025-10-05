import os
import zipfile
import subprocess

# --- Configuration ---
LIBRARY_DIR = "libraries/FourWD"  # Path to your Arduino library folder
ZIP_NAME = "FourWD.zip"           # Output zip file name
GITHUB_REPO = "Darainer/KidsCarArduinoControl"  # Set your repo here

# --- Get latest commit hash ---
# Uses git from command line, working directory set to library dir
def get_commit_hash(lib_dir):
    try:
        commit_hash = subprocess.check_output(
            ["git", "-C", lib_dir, "rev-parse", "HEAD"]
        ).decode().strip()
        return commit_hash
    except Exception as e:
        print(f"Error getting commit hash: {e}")
        return "UNKNOWN"

def make_github_info(lib_dir, commit_hash):
    github_url = f"https://github.com/{GITHUB_REPO}/commit/{commit_hash}"
    info_file = os.path.join(lib_dir, "GITHUB_INFO.txt")
    with open(info_file, "w") as f:
        f.write(f"Commit: {commit_hash}\n")
        f.write(f"Repo: {github_url}\n")
    return info_file

def zip_library(lib_dir, zip_name, info_file):
    # Create zip in parent dir of library
    parent_dir = os.path.dirname(lib_dir)
    zip_path = os.path.join(parent_dir, zip_name)
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for root, dirs, files in os.walk(lib_dir):
            # Exclude .git and its contents
            if ".git" in dirs:
                dirs.remove(".git")
            for file in files:
                if ".git" in file:
                    continue
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, parent_dir)
                zf.write(file_path, arcname)
    print(f"Created {zip_path} with GitHub info included.")

def cleanup(info_file):
    try:
        os.remove(info_file)
    except Exception as e:
        print(f"Cleanup failed: {e}")

if __name__ == "__main__":
    commit_hash = get_commit_hash(LIBRARY_DIR)
    info_file = make_github_info(LIBRARY_DIR, commit_hash)
    zip_library(LIBRARY_DIR, ZIP_NAME, info_file)
    cleanup(info_file)