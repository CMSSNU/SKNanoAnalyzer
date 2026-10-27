#include "FriendBuilder.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char *kUsage = R"(usage:
  sknano_friend build --central C.root --aux A1.root [--aux A2.root ...] --out F.root
                      --production NAME [--override COLL ...] [--exclude-prefix COLL ...]
                      [--mc] [--filtered] [--filter-note TEXT] [--scratch DIR] [--chunk N]
                      [--ntuple Events] [--report REPORT.json]
  sknano_friend check F.root C.root [--ntuple Events] [--report REPORT.json]

Prints the JSON report to stdout. Exit status 1 on any failure, 2 when a
check finds misaligned entries.
)";

std::string Need(const std::vector<std::string> &args, std::size_t &i, const std::string &flag) {
    if (i + 1 >= args.size()) {
        std::cerr << flag << " needs a value\n" << kUsage;
        std::exit(1);
    }
    return args[++i];
}

void Emit(const nlohmann::json &report, const std::string &path) {
    const auto text = report.dump(2);
    std::cout << text << std::endl;
    if (!path.empty()) {
        std::ofstream out(path);
        out << text << std::endl;
    }
}

} // namespace

int main(int argc, char **argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        std::cerr << kUsage;
        return 1;
    }
    try {
        if (args[0] == "build") {
            sknano_friend::BuildOptions options;
            std::string reportPath;
            for (std::size_t i = 1; i < args.size(); ++i) {
                const auto &flag = args[i];
                if (flag == "--central") options.central = Need(args, i, flag);
                else if (flag == "--aux") options.aux.push_back(Need(args, i, flag));
                else if (flag == "--out") options.out = Need(args, i, flag);
                else if (flag == "--production") options.production = Need(args, i, flag);
                else if (flag == "--override") options.overrides.push_back(Need(args, i, flag));
                else if (flag == "--exclude-prefix") options.excludePrefixes.push_back(Need(args, i, flag));
                else if (flag == "--mc") options.isMC = true;
                else if (flag == "--filter-note") options.filterNote = Need(args, i, flag);
                else if (flag == "--filtered") options.filtered = true;
                else if (flag == "--scratch") options.scratch = Need(args, i, flag);
                else if (flag == "--chunk") options.chunkEntries = std::stoul(Need(args, i, flag));
                else if (flag == "--ntuple") options.ntupleName = Need(args, i, flag);
                else if (flag == "--report") reportPath = Need(args, i, flag);
                else { std::cerr << "unknown option " << flag << "\n" << kUsage; return 1; }
            }
            if (options.central.empty() || options.out.empty() || options.production.empty()) {
                std::cerr << "--central, --out and --production are required\n" << kUsage;
                return 1;
            }
            Emit(sknano_friend::BuildFriend(options).toJson(), reportPath);
            return 0;
        }
        if (args[0] == "check") {
            if (args.size() < 3) {
                std::cerr << kUsage;
                return 1;
            }
            std::string ntuple = "Events", reportPath;
            for (std::size_t i = 3; i < args.size(); ++i) {
                const auto &flag = args[i];
                if (flag == "--ntuple") ntuple = Need(args, i, flag);
                else if (flag == "--report") reportPath = Need(args, i, flag);
                else { std::cerr << "unknown option " << flag << "\n" << kUsage; return 1; }
            }
            const auto report = sknano_friend::CheckFriend(args[1], args[2], ntuple);
            Emit(report.toJson(), reportPath);
            return report.mismatches == 0 ? 0 : 2;
        }
        std::cerr << kUsage;
        return 1;
    } catch (const std::exception &error) {
        std::cerr << "sknano_friend: " << error.what() << std::endl;
        return 1;
    }
}
