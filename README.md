# muniq

## Synopsis

muniq deduplicates emails in the current directory.

It needs to be run in the directory containing messages directly, like
Maildir/cur. The deduplication is done exclusively by the Message-ID header, so
any accidental Message-ID conflict will result in data loss.

Message-ID header key (but not value!) is case-insensitive.

## Usage

```sh
$ muniq --dryrun
```

Flags:

* `--dryrun`: doesn't actually remove any files
* `-h` or `--help`: prints a usage message and exits

## Build

D'oh!

```sh
$ gcc muniq.c -o muniq
```

This should build on any POSIX compliant system. Tested on MacOS and Raspbian.

You could also use provided bazel config:

```sh
$ bazel build :muniq
```

however this is an overkill for most applications. The bazel config is provided
so that this code can be easily reused in other bazel projects.

## No robots

No robots were harmed in making of this code. It was lovingly handcrafted by
a human with help from the Linux man pages.
