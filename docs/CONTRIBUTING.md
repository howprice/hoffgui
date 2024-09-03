# CONTRIBUTING

*Contributions will not be accepted until the project reaches ALPHA. Up until that point, the codebase is likely to change significantly and the project is not ready for external contributions.*

After ALPHA, the project will be open to contributions and...

Contributions are welcome. You may wish to fix bugs, add new features or improve the documentation. Please read the following guidelines before submitting a pull request.


# How to contribute

1. Search existing issues to avoid duplication.
2. Add any relevant information to existing issue, or open a new issue if required.
3. Fork and create a feature branch. One feature per branch please.
4. Make your changes, adhering to the coding conventions
5. Thoroughly test your changes
6. Squash all commits into a single commit
7. Submit a pull request, reference your original issue, and provide a concise description of how your changes fixed the issue

# Coding guidelines

## Do not use `strncpy`

This results in GCC release warning: 'strncpy' specified bound depends on the length of the source argument. Use `SafeStrncpy` from StringHelpers.h instead.

# Coding conventions

The project contains an [.editorconfig](.editorconfig) file to enforce some coding style. Otherwise, please try to make any code modification and additions in the style of the pre-existing code e.g. Naming conventions, capitalisation, spacing.

Do not optimise without profiling first to ensure that the changes are worthwhile. Iteration time is likely to be dominated by launching child processes.
