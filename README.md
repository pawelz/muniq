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

## No robots

No robots were harmed in making of this code. It was lovingly handcrafted by
a human with help from the Linux man pages.
