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
# CMC - Context Manager Collections
#
import contextlib
from typing import TypeVar, Iterable, Dict as _Dict, overload, Tuple, Sequence, Union, Optional
from typing import ItemsView, KeysView
from typing import Mapping, Type, List

from xju.assert_ import Assert
from collections import OrderedDict
od = OrderedDict

T = TypeVar('T', bound=contextlib.AbstractContextManager)

class CM(contextlib.AbstractContextManager):
    def __enter__(self):
        return self
    def __exit__(self, t, e, b) -> None:
        pass

# class decorator that adds context management __enter__ and __exit__
# that enter and exit all type-hinted attributes implementing context management
def cmclass(cls:Type[T]) -> Type[T]:
    resources:List[contextlib.ExitStack] = []
    base_classes_to_enter = [ base_class for base_class in cls.__bases__
                              if issubclass(base_class, contextlib.AbstractContextManager) ]
    attrs_to_enter = [ n for n, t in cls.__annotations__.items()
                         if issubclass(t, contextlib.AbstractContextManager)]
    def enter(self,
              base_classes_to_enter=base_classes_to_enter,
              attrs_to_enter=attrs_to_enter,
              resources=resources) -> Type[T]:
        with contextlib.ExitStack() as tentative:
            for base_class in base_classes_to_enter:
                cm = __ClassCm(base_class, self)
                tentative.enter_context(cm)
                pass
            for n in attrs_to_enter:
                tentative.enter_context(getattr(self, n))
                pass
            resources.append(tentative.pop_all())
        return self
    def exit(self, t, e, b, resources=resources) -> None:
        if resources is not None:
            with contextlib.closing(resources[0]):
                pass
            pass
        pass
    setattr(cls, '__enter__', enter)
    setattr(cls, '__exit__', exit)
    cls.__annotations__['__enter__'] = type(enter)
    cls.__annotations__['__exit__'] = type(exit)
    return cls

K = TypeVar('K')
V = TypeVar('V', bound=contextlib.AbstractContextManager)

# REVISIT: incomplete, untested, get all methods from help(dict), expand
# constructors like dict for strong typing
class Dict(Mapping[K, V], contextlib.AbstractContextManager):
    entered = False

    @overload
    def __init__(self):
        '''new empty Dict'''
        pass
    @overload
    def __init__(self, x:_Dict[K,V]):
        '''initialise with value from {x} assuming those values have not been "entered"'''
        pass
    @overload
    def __init__(self, x:Iterable[Tuple[K,V]]):
        '''initialise with value from {x} assuming those values have not been "entered"'''
        pass
    def __init__(self, *args, **kwargs):
        self.x:_Dict[K,V] = OrderedDict(*args, **kwargs)
        pass
    
    def __enter__(self):
        '''"enters" all values in order they were inserted'''
        with contextlib.ExitStack() as tentative:
            for k,v in self.x.items():
                tentative.enter_context(v)
            tentative.pop_all()
            pass
        self.entered = True
        return self

    def __exit__(self, t, e, b):
        '''"exits" all values in order they were inserted'''
        if self.entered:
            with contextlib.ExitStack() as resources:
                for k,v in self.x.items():
                    resources.push(v)
                    pass
                pass
            pass
        pass

    def __contains__(self, key:object) -> bool:
        return key in self.x

    def __delitem__(self, key:K):
        v = self.x[key]
        del self.x[key]
        if self.entered:
            with contextlib.ExitStack() as f:
                f.push(v)
                pass
            pass
        pass

    def __getitem__(self, key:K) -> V:
        return self.x[key]
    
    def __iter__(self):
        return iter(self.x.keys())

    def __len__(self) -> int:
        return len(self.x)

    def __repr__(self) -> str:
        return repr(self.x)

    def __setitem__(self, key:K, value:V) -> None:
        '''replace any current value of {key} with {value}'''
        '''- "enters" the new value and "exits" any old value'''
        old = self.x.get(key)
        if old is not value:
            self.x[key]=value
            if self.entered:
                if old is None:
                    value.__enter__()
                else:
                    with contextlib.ExitStack() as f:
                        f.push(old)
                    pass
                pass
            pass
        pass
    
    def __sizeof__(self) -> int:
        return self.x.__sizeof__()

    def clear(self):
        while len(self):
            self.popitem()
            pass
        pass

    def get(self, key, default=None):
        return self.x.get(key, default)

    def items(self) -> ItemsView[K,V]:
        '''items in order they were first inserted'''
        return self.x.items()

    def keys(self) -> KeysView[K]:
        '''keys in order they were first inserted'''
        return self.x.keys()

    @overload
    def pop(self, key:K) -> V:
        '''pop and "exit" value of {key} if self has a value for it
           otherwise raise KeyError'''
        pass
    @overload
    def pop(self, key:K, default:T) -> Union[V,T]:
        '''pop and "exit" value of {key} if self has a value for it
        otherwise return default'''
        pass
    def pop(self, key, default=None):
        if default is None:
            v = self.x.pop(key)
            if self.entered:
                with contextlib.ExitStack() as f:
                    f.push(v)
                pass
            return v
        else:
            try:
                v = self.x.pop(key)
                if self.entered:
                    with contextlib.ExitStack() as f:
                        f.push(v)
                    pass
                return v
            except KeyError:
                return default
            pass
        pass

    def popitem(self) -> Tuple[K, V]:
        '''pop and "exit" most recently added item'''
        k, v = self.x.popitem()
        if self.entered:
            with contextlib.ExitStack() as f:
                f.push(v)
                pass
            pass
        return k, v

    @overload
    def setdefault(self, key:K, default:V) -> V:
        '''set value of {key} to default "entered" if it has no value
           return value of {key}'''
        pass
    @overload
    def setdefault(self, key:K, default:None) -> Optional[V]:
        '''same as get(key,None)'''
        pass
    def setdefault(self, key, default):
        if default is not None:
            if not key in self:
                self[key] = default
            return self.x.get(key)
        else:
            return self.x.get(key, None)
        pass

    @overload
    def update(self, x:_Dict[K, V]):
        '''update self from {x} assuming its values are not yet entered'''
        pass
    @overload
    def update(self, x:Iterable[Tuple[K,V]]):
        '''update self from {x} assuming its values are not yet entered'''
        pass
    def update(self, x):
        if getattr(x,'keys',None):
            for k in x.keys():
                self[k] = x[k]
                pass
            pass
        else:
            for k,v in x:
                self[k] = v
                pass
            pass
        pass

class __ClassCm(contextlib.AbstractContextManager):
    '''target object {x} subclass {cls} context management methods
       - so for x: X where X(Y), __ClassCM(Y, x) will call Y.__enter__
         and Y.__exit__ directly avoiding any X.__enter__/__exit__
         overrides'''
    cls:Type
    x:object

    def __init__(self, cls, x):
        self.cls=cls
        self.x=x
        pass
    def __enter__(self):
        self.cls.__enter__(self.x)
        return self
    def __exit__(self, t, e, b):
        return self.cls.__exit__(self.x, t, e, b)
    pass

