import os

def dump_all_files(root_dir, output_file="output.txt"):
    with open(output_file, "w", encoding="utf-8") as out:

        # Walk through all subdirectories
        for dirpath, dirnames, filenames in os.walk(root_dir):
            for filename in filenames:
                file_path = os.path.join(dirpath, filename)
                if "/." in file_path:
                    continue

                out.write(f"\n===== FILE: {file_path} =====\n")

                try:
                    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                        out.write(f.read())
                except Exception as e:
                    out.write(f"<< Unable to read file: {e} >>\n")

    print(f"Done! Output written to: {output_file}")


dump_all_files(".", ".misc/source.txt")
