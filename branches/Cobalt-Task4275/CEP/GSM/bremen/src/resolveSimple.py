from math import log10
from src.resolve import BasicResolver


class SimpleResolver(BasicResolver):

    def load_sources(self, group_id):
        cursor = self.conn.get_cursor("""
select runcatid, wm_ra, wm_ra_err, wm_decl, wm_decl_err
  from runningcatalog r
 where exists (select 1 from temp_associations ta
                where ta.runcat_id = r.runcatid
                  and ta.group_head_id = %s)""" % (group_id))
        sources = cursor.fetchall()
        cursor.close()
        return sources

    def get_flux_distance(self, i1, i2):
        """
        Get de Ruiter distance in the coordinates + flux space.
        """
        dist = 1e-12
        for i in range(1, 4, 2):
            dist = dist + (i1[i] - i2[i])*(i1[i] - i2[i]) / \
                          (i1[i+1] * i1[i+1] + i2[i+1] * i2[i+1])
        return dist


    def resolve(self, detections, sources):
        solution = []
        is_ok = True
        if (len(detections) == len(sources)):
            source_minimum = [1e20]*len(sources)
            source_second = [1e20]*len(sources)
            source_isolation = [0.0]*len(sources)
            source_index = [0]*len(sources)
            detect_minimum = [1e20]*len(sources)
            detect_second = [1e20]*len(sources)
            detect_isolation = [0.0]*len(sources)
            detect_index = [0]*len(sources)
            for i, detect in enumerate(detections):
                for j, source in enumerate(sources):
                    dist = self.get_flux_distance(detect, source)
                    if dist < detect_minimum[i]:
                        detect_second[i] = detect_minimum[i]
                        detect_minimum[i] = dist
                        detect_isolation[i] = dist/detect_second[i]
                        detect_index[i] = j
                    if dist < source_minimum[j]:
                        source_second[j] = source_minimum[j]
                        source_minimum[j] = dist
                        source_isolation[j] = dist/source_second[j]
                        source_index[j] = i
            for i in xrange(len(detections)):
                if detect_isolation[i] < 0.02 and \
                   source_index[detect_index[i]] == i:
                    solution.append([detections[i][0], 
                                     sources[detect_index[i]][0]])
                else:
                    return False, []
        else:
            return False, []
        return is_ok, solution
                    
                        
            
        
