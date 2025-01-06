# Grid information used by remove_duplicate_files
resolutions = {
    'NS25': {
        'projections': ['N25', 'S25'],
        'ltods': ['E', 'M'],
        'file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'NS36': {
        'projections': ['N36', 'S36'],
        'ltods': ['E', 'M'],
        'file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'T25': {
        'projections': ['T25'],
        'ltods': ['A', 'D'],
        'file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'M36': {
        'projections': ['M36'],
        'ltods': ['A', 'D'],
        'file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_GRD_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'NS12.5': {
        'projections': ['N12.5', 'S12.5'],
        'ltods': ['E', 'M'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'NS09': {
        'projections': ['N09', 'S09'],
        'ltods': ['E', 'M'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'T12.5': {
        'projections': ['T12.5'],
        'ltods': ['A', 'D'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'M09': {
        'projections': ['M09'],
        'ltods': ['A', 'D'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'NS6.25': {
        'projections': ['N6.25', 'S6.25'],
        'ltods': ['E', 'M'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'T6.25': {
        'projections': ['T6.25'],
        'ltods': ['A', 'D'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'NS3.125': {
        'projections': ['N3.125', 'S3.125'],
        'ltods': ['E', 'M'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'NS03': {
        'projections': ['N03', 'S03'],
        'ltods': ['E', 'M'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'T3.125': {
        'projections': ['T3.125'],
        'ltods': ['A', 'D'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    },

    'M03': {
        'projections': ['M03'],
        'ltods': ['A', 'D'],
        'file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc',
        'premet_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.premet',
        'spatial_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.spatial',
        'met_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.met',
        'PDR_file_regex': '%s_SIR_EASE2_%skm_%s_%s_%s_%s_*.nc.PDR'
    }
}
