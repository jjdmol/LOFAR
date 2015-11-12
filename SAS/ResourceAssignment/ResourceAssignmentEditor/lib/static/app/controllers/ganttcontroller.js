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

    $scope.dataService = dataService;
    $scope.ganttData = []

    $scope.options = {
        mode: 'custom',
        scale: 'day',
        sideMode: 'Tree',
        columns: ['model.name', 'from', 'to'],
        treeTableColumns: ['from', 'to'],
        columnsHeaders: {'model.name' : 'Name', 'from': 'From', 'to': 'To'},
        columnsClasses: {'model.name' : 'gantt-column-name', 'from': 'gantt-column-from', 'to': 'gantt-column-to'},
        columnsFormatters: {
            'from': function(from) {
                return from !== undefined ? from.format('lll') : undefined;
            },
            'to': function(to) {
                return to !== undefined ? to.format('lll') : undefined;
            }
        },
        treeHeaderContent: '<i class="fa fa-align-justify"></i> {{getHeader()}}',
                              columnsHeaderContents: {
                                  'model.name': '<i class="fa fa-align-justify"></i> {{getHeader()}}',
                              'from': '<i class="fa fa-calendar"></i> {{getHeader()}}',
                              'to': '<i class="fa fa-calendar"></i> {{getHeader()}}'
                              },
        autoExpand: 'both',
        api: function(api) {
            // API Object is used to control methods and events from angular-gantt.
            $scope.api = api;
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
        var taskDict = $scope.dataService.taskDict;

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
                    var resourceRowId = 'group_' + groupIds[j] + '_resource_' + resource.id;
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
            var task = taskDict[groupClaim.taskId];

            var groupClaimTask = {
                name: task ? task.name : '<unknown>',
                'from': groupClaim.startTime,
                'to': groupClaim.endTime
            };

            var groupRowId = 'group_' + groupClaim.resourceGroupId;
            var ganntGroupRow = ganntRowsDict[groupRowId];
            if(ganntGroupRow)
                ganntGroupRow.tasks.push(groupClaimTask);
        }

        //and assign each resourceclaim to its resource in each group
        for(var i = 0; i < numResourceClaims; i++) {
            var claim = resourceClaims[i];
            var task = taskDict[claim.taskId];

            var groupIds = resourceIdToGroupIdsDict[claim.resourceId];
            var numGroups = groupIds.length;

            var claimTask = {
                name: task ? task.name : '<unknown>',
                'from': claim.startTime,
                'to': claim.endTime
            };

            if(numGroups > 0) {
                for(var j = 0; j < numGroups; j++) {
                    var resourceRowId = 'group_' + groupIds[j] + '_resource_' + claim.resourceId;
                    var ganntRow = ganntRowsDict[resourceRowId];
                    if(ganntRow)
                        ganntRow.tasks.push(claimTask);
                }
            }
            else {
                var resourceRowId = 'resource_' + claim.resourceId;
                var ganntRow = ganntRowsDict[resourceRowId];
                if(ganntRow)
                    ganntRow.tasks.push(claimTask);
            }
        }

        var ganntRows = [];

        for (var groupId in ganntRowsDict)
            ganntRows.push(ganntRowsDict[groupId]);

        $scope.ganttData = ganntRows;
        setTimeout($scope.api.tree.collapseAll(), 10)
    };

    $scope.$watch('dataService.tasks', updateGanttData);
    $scope.$watch('dataService.resources', updateGanttData);
    $scope.$watch('dataService.resourceclaims', updateGanttData);
    $scope.$watch('dataService.resourceGroups', updateGanttData);
    $scope.$watch('dataService.resourceGroupClaims', updateGanttData);
}
]);
