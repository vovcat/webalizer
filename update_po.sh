# Update webalizer.pot file
xgettext --default-domain=webalizer --directory=. -o po/webalizer.pot \
    --language=C --add-comments="TRANS:" --escape --sort-by-file \
    --keyword=_ --keyword=N_ --keyword=Q_ --keyword=PL_:1,2 *.c *.h

# Merge old .po files
for i in po/*.po; do echo -n $i; msgmerge -U $i po/webalizer.pot; done
rm -f po/*.po~;

# Update POT-Creation-Date/PO-Revision-Date headers
perl -MPOSIX -l -p -i -e 'BEGIN { $dt = $ENV{DT} or strftime(q|%F %H:%M%z|, localtime()); print qq|$dt\n|; } \
    s/^"POT-Creation-Date:.*$/"POT-Creation-Date: 2014-10-08 01:23+0200\\n"/; \
    s/^"PO-Revision-Date: [^Y].*$/"PO-Revision-Date: $dt\\n"/;' po/*.po*
