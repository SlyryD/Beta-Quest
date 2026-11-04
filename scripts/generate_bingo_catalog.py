#!/usr/bin/env python3

import argparse
import re
import subprocess
from pathlib import Path
from typing import List


EXPECTED_GOAL_COUNT = 255
DEFAULT_SOURCE_COMMIT = "d8068e992682834bb57713299d32c4d12ca5ff7b"


def parse_goals(source: Path) -> List[str]:
    text = source.read_text(encoding="utf-8")
    goals = re.findall(r'\{\s*name:\s*"((?:\\.|[^"])*)"\s*,\s*types:\s*\[[^\]]*\]\s*\}', text)
    goals = [bytes(goal, "utf-8").decode("unicode_escape") for goal in goals]
    if len(goals) != EXPECTED_GOAL_COUNT:
        raise ValueError(f"expected {EXPECTED_GOAL_COUNT} goals, found {len(goals)}")
    if len(set(goals)) != len(goals):
        raise ValueError("goal names are not unique")
    return goals


def goal_symbol(name: str) -> str:
    symbol = re.sub(r"[^A-Z0-9]+", "_", name.upper()).strip("_")
    if symbol[0].isdigit():
        symbol = "N_" + symbol
    return "BINGO_GOAL_" + symbol


def render_fixture(goals: List[str], commit: str) -> str:
    return "\n".join([f"# source_commit={commit}", *goals, ""])


def render_header(goals: List[str], commit: str) -> str:
    symbols = [goal_symbol(goal) for goal in goals]
    if len(set(symbols)) != len(symbols):
        duplicates = sorted(symbol for symbol in set(symbols) if symbols.count(symbol) > 1)
        raise ValueError(f"goal enum symbol collision: {', '.join(duplicates)}")

    enum_lines = []
    for index, (symbol, goal) in enumerate(zip(symbols, goals)):
        enum_lines.append(f"    {symbol} = {index}, // {goal}")

    return f"""#ifndef BINGO_H
#define BINGO_H

#include <stdbool.h>
#include <stdint.h>

#include "counts.h"

// Generated from ocarina_of_time_beta_quest_generator.js
// Source commit: {commit}
typedef enum {{
{chr(10).join(enum_lines)}
    BINGO_GOAL_COUNT = {len(goals)}
}} bingo_goal_t;

typedef struct {{
    uint16_t current;
    uint16_t target;
    bool complete;
}} bingo_progress_t;

bingo_progress_t bingo_get_goal_progress(bingo_goal_t goal);
bool bingo_goal_is_complete(bingo_goal_t goal);
uint16_t bingo_get_global_count(global_count_t count_index);
uint16_t bingo_get_dungeon_count(uint8_t dungeon_index, dungeon_count_t count_index);
uint16_t bingo_get_area_count(area_t area_index, area_count_t count_index);
uint8_t bingo_get_current_area_index(void);
uint8_t bingo_get_inventory_item(uint8_t slot);
uint8_t bingo_get_ammo(uint8_t slot);
uint8_t bingo_get_magic_level(void);
uint8_t bingo_get_strength_level(void);
uint8_t bingo_get_scale_level(void);
uint8_t bingo_get_capacity_level(uint8_t item);

bool bingo_item_owned(uint8_t item);

#endif
"""


def source_commit(source: Path) -> str:
    result = subprocess.run(
        ["git", "-C", str(source.parent), "rev-parse", "HEAD"],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return DEFAULT_SOURCE_COMMIT
    return result.stdout.strip()


def write_or_check(path: Path, content: str, check: bool) -> None:
    if check:
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            raise ValueError(f"{path} is out of date")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def check_implementation(path: Path, goals: List[str]) -> None:
    text = path.read_text(encoding="utf-8")
    expected = {goal_symbol(goal) for goal in goals}
    implemented = set(re.findall(r"case\s+(BINGO_GOAL_[A-Z0-9_]+)\s*:", text))
    implemented.discard("BINGO_GOAL_COUNT")
    missing = sorted(expected - implemented)
    unexpected = sorted(implemented - expected)
    if missing or unexpected:
        details = []
        if missing:
            details.append("missing " + ", ".join(missing))
        if unexpected:
            details.append("unexpected " + ", ".join(unexpected))
        raise ValueError(f"{path} evaluator coverage mismatch: {'; '.join(details)}")


def render_report(goals: List[str], commit: str) -> str:
    rows = []
    for index, goal in enumerate(goals):
        rows.append(
            f"<tr><td>{index}</td><td>{goal}</td><td><code>{goal_symbol(goal)}</code></td>"
            "<td>Centralized</td></tr>"
        )
    return f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Beta Quest Bingo Coverage</title>
<style>
body {{ background:#10141c; color:#eef3ff; font:14px system-ui,sans-serif; margin:24px; }}
code {{ color:#9ed0ff; }}
table {{ border-collapse:collapse; width:100%; }}
th,td {{ border:1px solid #3d4658; padding:6px 8px; text-align:left; }}
th {{ background:#20283a; position:sticky; top:0; }}
tr:nth-child(even) {{ background:#171d2a; }}
.summary {{ color:#a9f5b4; font-weight:700; }}
</style>
</head>
<body>
<h1>Beta Quest Bingo Coverage</h1>
<p>Source commit: <code>{commit}</code></p>
<p class="summary">{len(goals)} / {len(goals)} goals have centralized evaluators.</p>
<table>
<thead><tr><th>ID</th><th>Goal</th><th>Stable symbol</th><th>Coverage</th></tr></thead>
<tbody>
{chr(10).join(rows)}
</tbody>
</table>
</body>
</html>
"""


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path)
    parser.add_argument("--fixture", type=Path, default=Path("tests/data/bingo_goals.txt"))
    parser.add_argument("--header", type=Path, default=Path("c/bingo.h"))
    parser.add_argument("--implementation", type=Path, default=Path("c/bingo.c"))
    parser.add_argument("--report", type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    if args.source:
        goals = parse_goals(args.source)
        commit = source_commit(args.source)
        fixture = render_fixture(goals, commit)
    else:
        lines = args.fixture.read_text(encoding="utf-8").splitlines()
        if not lines or not lines[0].startswith("# source_commit="):
            raise ValueError("fixture is missing its source commit")
        commit = lines[0].split("=", 1)[1]
        goals = lines[1:]
        if len(goals) != EXPECTED_GOAL_COUNT:
            raise ValueError(f"expected {EXPECTED_GOAL_COUNT} fixture goals, found {len(goals)}")
        fixture = render_fixture(goals, commit)

    write_or_check(args.fixture, fixture, args.check)
    write_or_check(args.header, render_header(goals, commit), args.check)
    if args.check:
        check_implementation(args.implementation, goals)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(render_report(goals, commit), encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
