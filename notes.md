# General
- Would have been nice to have some sort of visualization to validate the algorithm
- How is the clustering done? you are just shuffling around data (which has just one dimension). I am missing something? It seems to me the whole core of the project is missing
- Good repository structure, files are in predictable folder, readme is short but complete, nice comments and formatting

# Code
## Major
- It is very limiting that the algorithm works just with 4 processors
- There are some issues in the design of the classes
- Should not use `new` if not strictly necessary, you should prefer to use managed memory, like std::vector

## Minor
- There are few misunderstandings concerning the usage of header guards and including files