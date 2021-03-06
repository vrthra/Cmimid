# -*- coding: utf-8 -*-
#
# Copyright (c) 2015 Jonathan M. Lange <jml@mumak.net>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


"""Library for formatting trees."""

import itertools


class Options(object):

    def __init__(self,
                 FORK=u'\u251c',
                 LAST=u'\u2514',
                 VERTICAL=u'\u2502',
                 HORIZONTAL=u'\u2500',
                 NEWLINE=u'\u23ce'):
        self.FORK = FORK
        self.LAST = LAST
        self.VERTICAL = VERTICAL
        self.HORIZONTAL = HORIZONTAL
        self.NEWLINE = NEWLINE


ASCII_OPTIONS = Options(FORK=u'|',
                        LAST=u'+',
                        VERTICAL=u'|',
                        HORIZONTAL=u'-',
                        NEWLINE=u'\n')


def _format_newlines(prefix, formatted_node, options):
    """
    Convert newlines into U+23EC characters, followed by an actual newline and
    then a tree prefix so as to position the remaining text under the previous
    line.
    """
    replacement = u''.join([
        options.NEWLINE,
        u'\n',
        prefix])
    return formatted_node.replace(u'\n', replacement)


def _format_tree(node, format_node, get_children, options, prefix=u''):
    children = list(get_children(node))
    next_prefix = u''.join([prefix, options.VERTICAL, u'   '])
    for child in children[:-1]:
        yield u''.join([prefix,
                        options.FORK,
                        options.HORIZONTAL,
                        options.HORIZONTAL,
                        u' ',
                        _format_newlines(next_prefix,
                                         format_node(child),
                                         options)])
        for result in _format_tree(child,
                                   format_node,
                                   get_children,
                                   options,
                                   next_prefix):
            yield result
    if children:
        last_prefix = u''.join([prefix, u'    '])
        yield u''.join([prefix,
                        options.LAST,
                        options.HORIZONTAL,
                        options.HORIZONTAL,
                        u' ',
                        _format_newlines(last_prefix,
                                         format_node(children[-1]),
                                         options)])
        for result in _format_tree(children[-1],
                                   format_node,
                                   get_children,
                                   options,
                                   last_prefix):
            yield result


def format_tree(node, format_node, get_children, options=None):
    lines = itertools.chain(
        [format_node(node)],
        _format_tree(node, format_node, get_children, options or Options()),
        [u''],
    )
    return u'\n'.join(lines)


def format_ascii_tree(tree, format_node, get_children):
    """ Formats the tree using only ascii characters """
    return format_tree(tree,
                       format_node,
                       get_children,
                       ASCII_OPTIONS)


def print_tree(*args, **kwargs):
    print(format_tree(*args, **kwargs))

if __name__ == '__main__':
    import sys
    import json
    jsonfn = sys.argv[1]
    with open(jsonfn) as f:
        jsont = json.load(f)
    trees = range(len(jsont))
    if len(sys.argv) > 2:
        trees = [int(v) for v in sys.argv[2:]]
    for tree in trees:
        print_tree(jsont[tree]['tree'], format_node=lambda x: repr(x[0]), get_children=lambda x: x[1])
