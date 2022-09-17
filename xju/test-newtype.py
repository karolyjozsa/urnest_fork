# Copyright (c) 2022 Trevor Taylor
# coding: utf-8
# 
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that all
# copyright notices and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

from xju.newtype import stringlike,RunTimeStr
from xju.assert_ import Assert

@stringlike
class FirstName(RunTimeStr):
    pass

@stringlike
class LastName(RunTimeStr):
    pass


def full_name(first_name:FirstName,last_name:LastName)->str:
    return f'{first_name} {last_name}'

first_name=FirstName('fred')
last_name=LastName('jones')

Assert(full_name(first_name,last_name))=='fred jones'
Assert(first_name).startsWith('fr')
