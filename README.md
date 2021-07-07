# gpkg-merger

Merge seperate gpkgs to one gpkg

Set VACUUM_COUNT env variable to define the amount of tiles that should be merged before a vacuum should occur.
Default value is 4000000 (4M).

Example:
export VACUUM_COUNT=10000
(executes the vacuum command every 10000 tiles)
