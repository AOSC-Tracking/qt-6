# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import abc
import logging
from typing import List, Optional, Sequence, Tuple, Type, TypeVar

from crossbench.runner.run import Run

from .story import Story

PressBenchmarkStoryT = TypeVar(
    "PressBenchmarkStoryT", bound="PressBenchmarkStory")


class PressBenchmarkStory(Story, metaclass=abc.ABCMeta):
  NAME: str = ""
  URL: str = ""
  URL_LOCAL: str = ""
  SUBSTORIES: Tuple[str, ...] = ()

  @classmethod
  def all_story_names(cls) -> Tuple[str, ...]:
    assert cls.SUBSTORIES
    return cls.SUBSTORIES

  @classmethod
  def default_story_names(cls) -> Tuple[str, ...]:
    """Override this method to use a subset of all_story_names as default
    selection if no story names are provided."""
    return cls.all_story_names()

  @classmethod
  def all(cls: Type[PressBenchmarkStoryT],
          separate: bool = False,
          url: Optional[str] = None,
          **kwargs) -> List[PressBenchmarkStoryT]:
    return cls.from_names(cls.all_story_names(), separate, url, **kwargs)

  @classmethod
  def default(cls: Type[PressBenchmarkStoryT],
              separate: bool = False,
              url: Optional[str] = None,
              **kwargs) -> List[PressBenchmarkStoryT]:
    return cls.from_names(cls.default_story_names(), separate, url, **kwargs)

  @classmethod
  def from_names(cls: Type[PressBenchmarkStoryT],
                 substories: Sequence[str],
                 separate: bool = False,
                 url: Optional[str] = None,
                 **kwargs) -> List[PressBenchmarkStoryT]:
    if not substories:
      raise ValueError("No substories provided")
    if separate:
      return [
          cls(  # pytype: disable=not-instantiable
              url=url,
              substories=[substory],
              **kwargs) for substory in substories
      ]
    return [
        cls(  # pytype: disable=not-instantiable
            url=url,
            substories=substories,
            **kwargs)
    ]

  _substories: Sequence[str]
  _url: str

  def __init__(self,
               *args,
               substories: Sequence[str] = (),
               duration: Optional[float] = None,
               url: Optional[str] = None,
               **kwargs) -> None:
    cls = self.__class__
    assert self.SUBSTORIES, f"{cls}.SUBSTORIES is not set."
    assert self.NAME is not None, f"{cls}.NAME is not set."
    self._verify_url(self.URL, "URL")
    self._verify_url(self.URL_LOCAL, "URL_LOCAL")
    assert substories, f"No substories provided for {cls}"
    self._substories = substories
    self._verify_substories()
    kwargs["name"] = self._get_unique_name()
    kwargs["duration"] = duration or self._get_initial_duration()
    super().__init__(*args, **kwargs)
    if not url:
      self._url = self.URL
    else:
      self._url = url
    assert self._url, f"Invalid URL for '{self.NAME}' in {type(self)}"

  def _get_unique_name(self) -> str:
    substories_set = set(self._substories)
    if substories_set == set(self.default_story_names()):
      return self.NAME
    if substories_set == set(self.all_story_names()):
      name = f"{self.NAME}_all"
    else:
      name = f"{self.NAME}_" + ("_".join(self._substories))
    if len(name) > 220:
      # Crop the name and add some random hash bits
      name = name[:220] + hex(hash(name))[2:10]
    return name

  def _get_initial_duration(self) -> dt.timedelta:
    # Fixed delay for startup costs
    startup_delay = dt.timedelta(seconds=2)
    # Add some slack due to different story lengths
    story_factor = 0.5 + 1.1 * len(self._substories)
    return startup_delay + (story_factor * self.substory_duration)

  @property
  def substories(self) -> List[str]:
    return list(self._substories)

  @property
  def fast_duration(self) -> dt.timedelta:
    """Expected benchmark duration on fast machines.
    Keep this low enough to not have to wait needlessly at the end of a
    benchmark.
    """
    return self.duration / 3

  @property
  def slow_duration(self) -> dt.timedelta:
    """Max duration that covers run-times on slow machines and/or
    debug-mode browsers.
    Making this number too large might cause needless wait times on broken
    browsers/benchmarks.
    """
    return dt.timedelta(seconds=15) + self.duration * 5

  @property
  @abc.abstractmethod
  def substory_duration(self) -> dt.timedelta:
    pass

  @property
  def url(self) -> str:
    return self._url

  def _verify_url(self, url: str, property_name: str) -> None:
    cls = self.__class__
    assert url is not None, f"{cls}.{property_name} is not set."

  def _verify_substories(self) -> None:
    if len(self._substories) != len(set(self._substories)):
      # Beware of the O(n**2):
      duplicates = set(
          substory for substory in self._substories
          if self._substories.count(substory) > 1)
      assert duplicates, (
          f"substories='{self._substories}' contains duplicate entries: "
          f"{duplicates}")
    if self._substories == self.SUBSTORIES:
      return
    for substory in self._substories:
      assert substory in self.SUBSTORIES, (f"Unknown {self.NAME} substory %s" %
                                           substory)

  def log_run_details(self, run: Run) -> None:
    super().log_run_details(run)
    self.log_run_test_url(run)

  def log_run_test_url(self, run: Run):
    del run
    logging.info("STORY PUBLIC TEST URL: %s", self.URL)
