// $Id: ganttcontroller.js 32761 2015-11-02 11:50:21Z schaap $

var ganttControllerMod = angular.module('GanttControllerMod', [
                                        'gantt',
                                        'gantt.sortable',
                                        'gantt.movable',
                                        'gantt.drawtask',
                                        'gantt.tooltips',
                                        'gantt.bounds',
                                        'gantt.progress',
                                        'gantt.table',
                                        'gantt.tree',
                                        'gantt.groups',
                                        'gantt.overlap',
                                        'gantt.resizeSensor']).config(['$compileProvider', function($compileProvider) {
    $compileProvider.debugInfoEnabled(false); // Remove debug info (angularJS >= 1.3)
}]);

ganttControllerMod.controller('GanttController', ['$scope', 'dataService', function($scope, dataService) {

    var self = this;
    self.doInitialCollapse = true;

    $scope.dataService = dataService;
    $scope.ganttData = []

    self.taskStatusColors = {'active':'#00ff00', 'aborted':'#ff0000', 'scheduled':'#ffff00', 'prescheduled':'#cccc00' }

    $scope.options = {
        mode: 'custom',
        scale: 'day',
        sideMode: 'Tree',
        columns: ['model.name', 'starttime', 'endtime'],
        treeTableColumns: ['starttime', 'endtime'],
        columnsHeaders: {'model.name' : 'Name', 'starttime': 'From', 'endtime': 'To'},
        columnsClasses: {'model.name' : 'gantt-column-name', 'starttime': 'gantt-column-from', 'endtime': 'gantt-column-to'},
        columnsFormatters: {
            'starttime': function(starttime) {
                return starttime !== undefined ? starttime.format('lll') : undefined;
            },
            'endtime': function(endtime) {
                return endtime !== undefined ? endtime.format('lll') : undefined;
            }
        },
        treeHeaderContent: '<i class="fa fa-align-justify"></i> {{getHeader()}}',
                              columnsHeaderContents: {
                                  'model.name': '<i class="fa fa-align-justify"></i> {{getHeader()}}',
                              'starttime': '<i class="fa fa-calendar"></i> {{getHeader()}}',
                              'endtime': '<i class="fa fa-calendar"></i> {{getHeader()}}'
                              },
        autoExpand: 'both',
        api: function(api) {
            // API Object is used to control methods and events from angular-gantt.
            $scope.api = api;

            api.core.on.ready($scope, function () {
                    api.tasks.on.moveEnd($scope, moveHandler);
                    api.tasks.on.resizeEnd($scope, moveHandler);
                }
            );
        }
    };

    function moveHandler(item)
    {
        var task_id = undefined;
        if(item.row.model.id.startsWith('group')) {
            var claimGroupId = item.model.id;
            var claimGroup = $scope.dataService.resourceGroupClaimDict[claimGroupId];
            task_id = claimGroup.task_id;
        }
        else {
            var claimId = item.model.id;
            var claim = $scope.dataService.resourceClaimDict[claimId];
            task_id = claim.task_id;
        }
        if(task_id) {
            var task = $scope.dataService.taskDict[task_id];
            task.starttime = item.model.starttime._d.toISOString();
            task.endtime = item.model.endtime._d.toISOString();
            $scope.dataService.putTask(task);
        }
    };

    function updateGanttData() {
        var ganntRowsDict = {};

        var resourceGroups = $scope.dataService.resourceGroups;
        var numResourceGroups = resourceGroups.length;

        var resourceIdToGroupIdsDict = $scope.dataService.resourceIdToGroupIdsDict;
        var resources = $scope.dataService.resources;
        var numResources = resources.length;

        var resourceGroupsDict = $scope.dataService.resourceGroupsDict;
        var taskDict = $scope.dataService.filteredTaskDict;

        var resourceGroupClaims = $scope.dataService.resourceGroupClaims;
        var numResourceGroupClaims = resourceGroupClaims.length;

        var resourceClaims = $scope.dataService.resourceClaims;
        var numResourceClaims = resourceClaims.length;

        var resourceDict = $scope.dataService.resourceDict;

        //create rows in gantt for resourceGroups
        for(var i = 0; i < numResourceGroups; i++)
        {
            var resourceGroup = resourceGroups[i];
            var groupRowId = 'group_' + resourceGroup.id;
            ganntRowsDict[groupRowId] = {
                'id': groupRowId,
                'name': resourceGroup.name,
                'tasks': []
            };
        }

        //create rows in gantt for resources
        //add each resource row to all group rows of which group it is a member
        for(var i = 0; i < numResources; i++)
        {
            var resource = resources[i];
            var groupIds = resourceIdToGroupIdsDict[resource.id];
            var numGroups = groupIds.length;

            if(numGroups > 0) {
                for(var j = 0; j < numGroups; j++) {
                    var parentRowId = 'group_' + groupIds[j];
                    var resourceRowId = 'resource_' + resource.id + '_group_' + groupIds[j];
                    //make resource row child of group row
                    ganntRowsDict[resourceRowId] = {
                        'id': resourceRowId,
                        'parent': parentRowId,
                        'name': resource.name,
                        'tasks': []
                    };
                }
            }
            else
            {
                //no parent groups, so one individual row
                var resourceRowId = 'resource_' + resource.id;
                ganntRowsDict[resourceRowId] = {
                    'id': resourceRowId,
                    'name': resource.name,
                    'tasks': []
                };
            }
        }

        //now that we have all rows for the gantt...
        //assign each groupclaim to its resourcegroup
        for(var i = 0; i < numResourceGroupClaims; i++) {
            var groupClaim = resourceGroupClaims[i];
            var task = taskDict[groupClaim.task_id];

            if(task)
            {
                var groupRowId = 'group_' + groupClaim.resourceGroupId;
                var ganntGroupRow = ganntRowsDict[groupRowId];

                var groupClaimTask = {
                    id: groupClaim.id,
                    name: task ? task.name : '<unknown>',
                    'from': groupClaim.starttime,
                    'to': groupClaim.endtime,
                    'color': self.taskStatusColors[task.status]
                };

                if(ganntGroupRow)
                    ganntGroupRow.tasks.push(groupClaimTask);
            }
        }

        //and assign each resourceclaim to its resource in each group
        for(var i = 0; i < numResourceClaims; i++) {
            var claim = resourceClaims[i];
            var task = taskDict[claim.task_id];

            if(task)
            {
                var groupIds = resourceIdToGroupIdsDict[claim.resource_id];
                var numGroups = groupIds.length;

                if(numGroups > 0) {
                    for(var j = 0; j < numGroups; j++) {
                        var resourceRowId = 'resource_' + claim.resource_id + '_group_' + groupIds[j];
                        var ganntRow = ganntRowsDict[resourceRowId];
                        if(ganntRow)
                        {
                            var claimTask = {
                                id: claim.id,
                                name: task ? task.name : '<unknown>',
                                'from': claim.starttime,
                                'to': claim.endtime,
                                'color': self.taskStatusColors[task.status]
                            };

                            ganntRow.tasks.push(claimTask);
                        }
                    }
                }
                else {
                    var resourceRowId = 'resource_' + claim.resource_id;
                    var ganntRow = ganntRowsDict[resourceRowId];
                    if(ganntRow)
                    {
                        var claimTask = {
                            name: task ? task.name : '<unknown>',
                            'from': claim.starttime,
                            'to': claim.endtime,
                            'color': self.taskStatusColors[task.status]
                        };

                        ganntRow.tasks.push(claimTask);
                    }
                }
            }
        }

        var ganntRows = [];

        for (var groupId in ganntRowsDict)
            ganntRows.push(ganntRowsDict[groupId]);

        $scope.ganttData = ganntRows;

        if(self.doInitialCollapse && numResources && numResourceGroups)
        {
            doInitialCollapse = false;
//             setTimeout(function() { $scope.api.tree.collapseAll(); }, 50);
        }
    };

    $scope.$watch('dataService.tasks', updateGanttData, true);
    $scope.$watch('dataService.resources', updateGanttData);
    $scope.$watch('dataService.resourceClaims', updateGanttData, true);
    $scope.$watch('dataService.resourceGroups', updateGanttData);
    $scope.$watch('dataService.resourceGroupClaims', updateGanttData, true);
    $scope.$watch('dataService.filteredTaskDict', updateGanttData);
}
]);
