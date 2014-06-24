#!/usr/bin/python

class Grouper(object):
    """
    Group finder for GSM.
    Takes a list of tuples in the form:
    (xtrsrc_id, runcat_id, group_head_id).
    One cycle puts items from one group into two sets:
    xtrsrcset for items from the extractedsources,
    runcatset for items from the runningcatalog
    """

    def __init__(self, alist):
        """---"""
        self.alist = alist
        self.xtrsrcset = set()
        self.runcatset = set()
        self.group = None
        self.processed = 0

    def get_nodes_runcat(self, root, exclude=set()):
        """
        Get all records with root as runcatid, exluding those linked to
        exclude set.
        """
        for xtrsrcid, runcatid, groupid in self.alist:
            if runcatid == root and xtrsrcid not in exclude:
                yield (xtrsrcid, runcatid, groupid)

    def get_nodes_xtrsrc(self, root, exclude=set()):
        """
        Get all records with root as xtrsrcid, exluding those linked to
        exclude set.
        """
        for xtrsrcid, runcatid, groupid in self.alist:
            if xtrsrcid == root and runcatid not in exclude:
                yield (xtrsrcid, runcatid, groupid)

    def cycle_nodes_xtrsrc(self):
        """
        Add missing nodes to runcatset using xtrsrcset.
        """
        for it in self.xtrsrcset:
            for _, runcatid, _ in self.get_nodes_xtrsrc(it, self.runcatset):
                self.processed = self.processed + 1
                self.runcatset.add(runcatid)

    def cycle_nodes_runcat(self):
        """
        Add missing nodes to xtrsrcset using runcatset.
        """
        for it in self.runcatset:
            for xtrsrcid, _, _ in self.get_nodes_runcat(it, self.xtrsrcset):
                self.processed = self.processed + 1
                self.xtrsrcset.add(xtrsrcid)

    def one_cycle(self):
        """
        Make one group-detection cycle.
        """
        self.xtrsrcset = set()
        if self.alist and self.alist[0]:
            self.xtrsrcset.add(self.alist[0][0])
        self.runcatset = set()
        self.processed = 0
        old_processed = -1
        self.group = self.alist[0][2]
        while old_processed < self.processed:
            old_processed = self.processed
            self.cycle_nodes_xtrsrc()
            self.cycle_nodes_runcat()

    def cleanup(self):
        """
        Remove items from the detected group from the list.
        """
        newlist = []
        for ggg in xrange(len(self.alist)):
            if self.alist[ggg][0] not in self.xtrsrcset:
                newlist.append(self.alist[ggg])
        self.alist = newlist

    def is_completed(self):
        """
        True if there are undiscovered groups left.
        """
        return len(self.alist) > 0
